import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest import mock


MODULE_PATH = Path(__file__).resolve().parents[1] / "tools/research/optimizer_matrix.py"
SPEC = importlib.util.spec_from_file_location("optimizer_matrix", MODULE_PATH)
matrix = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(matrix)


def write_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value))


def fixture_root():
    root = Path(tempfile.mkdtemp(prefix="optimizer-matrix-test-"))
    starts = 0x100
    regions, candidates = [], []
    for kind, source, expected_names in matrix.SOURCES:
        root.joinpath(source).parent.mkdir(parents=True, exist_ok=True)
        root.joinpath(source).write_text(f"/* fixture: {source} */\n")
        names = list(expected_names)
        target = candidates if kind == "candidate" else regions
        for name in names:
            target.append({"name": name, "start": starts, "end": starts + 2,
                           "source": source, "mode": "thumb", "status": "matching_c"})
            starts += 2
        starts += 0x20
    write_json(root / "config/decomp_regions.json", {"functions": regions})
    write_json(root / "config/decomp_candidates.json", {"functions": candidates})
    write_json(root / "config/toolchain.json", {"compiler_banner": "fixture gcc 2.9", "revision": "fixture"})
    ref = root / matrix.REFERENCE
    ref.parent.mkdir(parents=True, exist_ok=True)
    ref.write_bytes(b"immutable reference fixture")
    compiler = root / matrix.COMPILER_BINARY
    compiler.parent.mkdir(parents=True, exist_ok=True)
    compiler.write_bytes(b"immutable compiler fixture")
    return root


def fake_report(descriptor, opt, *, diff=False):
    symbols = []
    for item in descriptor["symbols"]:
        symbols.append({
            "name": item["name"], "status": "DIFF" if diff else "MATCH",
            "matches": not diff, "expected_size": item["size"],
            "actual_size": item["size"] + (1 if diff else 0),
            "differing_bytes": 1 if diff else 0, "reference_start": item["start"],
            "actual_start": item["start"], "address_delta": 0,
            "layout_matches": not diff,
        })
    block_size = descriptor["end"] - descriptor["start"]
    return {
        "optimization": opt, "functions": symbols, "accepted": not diff,
        "matches": not diff, "expected_size": block_size,
        "actual_size": block_size + (1 if diff else 0), "differing_bytes": 1 if diff else 0,
    }


class DescriptorTests(unittest.TestCase):
    def test_plan_uses_manifest_integer_bounds_and_explicit_padding(self):
        root = fixture_root()
        try:
            plan = matrix.descriptor_plan(root)
            self.assertEqual(len(plan), 8)
            for descriptor in plan:
                self.assertEqual(descriptor["start_hex"], hex(descriptor["start"]))
                self.assertEqual(descriptor["end_hex"], hex(descriptor["end"]))
                self.assertEqual(descriptor["source_sha256"], hashlib.sha256(
                    (root / descriptor["source"]).read_bytes()).hexdigest())
            suffixes = {d["source"]: d["alignment_suffix"] for d in plan}
            self.assertEqual(suffixes["src/nonmatching/tree_detach.c"], 2)
            self.assertEqual(suffixes["src/list_callbacks.c"], 2)
            self.assertEqual(suffixes["src/content_index.c"], 0)
        finally:
            import shutil
            shutil.rmtree(root)

    def test_rejects_manifest_source_mismatch_and_noncontiguous_tu(self):
        root = fixture_root()
        try:
            path = root / "config/decomp_regions.json"
            data = json.loads(path.read_text())
            data["functions"][0]["source"] = "src/elsewhere.c"
            write_json(path, data)
            with self.assertRaises(matrix.MatrixError):
                matrix.descriptor_plan(root)
        finally:
            import shutil
            shutil.rmtree(root)

    def test_rejects_unexpected_or_missing_probe_symbols_and_metrics(self):
        root = fixture_root()
        try:
            descriptor = matrix.descriptor_plan(root)[0]
            report = fake_report(descriptor, "2")
            report["functions"][0].pop("address_delta")
            with self.assertRaises(matrix.MatrixError):
                matrix.normalize_probe(report, descriptor, "2")
            report = fake_report(descriptor, "2")
            report["functions"].append(dict(report["functions"][0]))
            with self.assertRaises(matrix.MatrixError):
                matrix.normalize_probe(report, descriptor, "2")
            report = fake_report(descriptor, "2")
            report.pop("differing_bytes")
            with self.assertRaises(matrix.MatrixError):
                matrix.normalize_probe(report, descriptor, "2")
            report = fake_report(descriptor, "2")
            report["functions"][0]["differing_bytes"] = 5
            with self.assertRaisesRegex(matrix.MatrixError, "byte equality"):
                matrix.normalize_probe(report, descriptor, "2")
            report = fake_report(descriptor, "2")
            report["functions"][0]["address_delta"] = True
            with self.assertRaisesRegex(matrix.MatrixError, "invalid symbol metrics"):
                matrix.normalize_probe(report, descriptor, "2")
        finally:
            import shutil
            shutil.rmtree(root)

    def test_reference_hash_must_match_immutable_snapshot(self):
        root = fixture_root()
        try:
            digest = hashlib.sha256((root / matrix.REFERENCE).read_bytes()).hexdigest()
            with mock.patch.object(matrix, "REFERENCE_SHA256", digest):
                self.assertEqual(matrix.validate_reference(root), digest)
                (root / matrix.REFERENCE).write_bytes(b"changed")
                with self.assertRaises(matrix.MatrixError):
                    matrix.validate_reference(root)
        finally:
            import shutil
            shutil.rmtree(root)


class ExecutionTests(unittest.TestCase):
    def setUp(self):
        self.root = fixture_root()
        self.reference_digest = hashlib.sha256((self.root / matrix.REFERENCE).read_bytes()).hexdigest()
        self.hash_patch = mock.patch.object(matrix, "REFERENCE_SHA256", self.reference_digest)
        self.hash_patch.start()

    def tearDown(self):
        self.hash_patch.stop()
        import shutil
        shutil.rmtree(self.root)

    def install_fake_probe(self, *, mode="match"):
        plan = matrix.descriptor_plan(self.root)
        by_index = {i: desc for i, desc in enumerate(plan)}
        calls = []

        def run(command, cwd, capture_output, text, env):
            calls.append(command)
            source = Path(command[2])
            stem = source.stem
            index = int(stem.split("_")[1])
            opt = command[command.index("--opt") + 1]
            output = self.root / "build/probes" / stem
            output.mkdir(parents=True, exist_ok=False)
            if mode == "missing":
                return subprocess.CompletedProcess(command, 1, stdout="stale-looking stdout", stderr="")
            preprocessed = f"preprocessed:{by_index[index]['source']}"
            if mode == "changing_preprocessed":
                preprocessed += f":opt={opt}"
            (output / "candidate.i").write_text(preprocessed)
            report = fake_report(by_index[index], opt, diff=(mode == "diff"))
            (output / "comparison.json").write_text(json.dumps(report))
            return subprocess.CompletedProcess(command, 1 if mode == "diff" else 0,
                                                stdout="tool chatter that must not enter JSON", stderr="")

        return calls, run

    def test_complete_matrix_has_deterministic_order_and_no_tool_stdout(self):
        calls, fake = self.install_fake_probe()
        source_hashes = {path: hashlib.sha256((self.root / path).read_bytes()).hexdigest()
                         for _, path, _ in matrix.SOURCES}
        with mock.patch.object(matrix.subprocess, "run", side_effect=fake):
            result = matrix.run_matrix(self.root)
        self.assertEqual(source_hashes, {path: hashlib.sha256((self.root / path).read_bytes()).hexdigest()
                                         for _, path, _ in matrix.SOURCES})
        self.assertEqual(len(calls), 32)
        self.assertEqual([row["optimization"] for row in result["results"][:4]], ["0", "1", "2", "s"])
        self.assertEqual(result["aggregate_by_optimization"]["2"]["translation_units"], 8)
        self.assertEqual(result["reference_elf_sha256"], self.reference_digest)
        self.assertEqual(result["compiler"]["binary_sha256"], hashlib.sha256(
            (self.root / matrix.COMPILER_BINARY).read_bytes()).hexdigest())
        self.assertTrue(all(row["preprocessed_sha256"] for row in result["results"]))
        self.assertNotIn("tool chatter", json.dumps(result))
        with mock.patch.object(matrix.subprocess, "run", side_effect=fake):
            second = matrix.run_matrix(self.root)
        self.assertEqual(result, second)

    def test_valid_diff_exit_is_kept_as_evidence(self):
        _, fake = self.install_fake_probe(mode="diff")
        with mock.patch.object(matrix.subprocess, "run", side_effect=fake):
            result = matrix.run_matrix(self.root)
        self.assertTrue(all(row["symbols"][0]["status"] == "DIFF" for row in result["results"]))

    def test_missing_fresh_result_fails_closed(self):
        _, fake = self.install_fake_probe(mode="missing")
        with mock.patch.object(matrix.subprocess, "run", side_effect=fake):
            with self.assertRaisesRegex(matrix.MatrixError, "no fresh comparison.json"):
                matrix.run_matrix(self.root)

    def test_header_change_between_optimization_levels_fails_closed(self):
        _, fake = self.install_fake_probe(mode="changing_preprocessed")
        with mock.patch.object(matrix.subprocess, "run", side_effect=fake):
            with self.assertRaisesRegex(matrix.MatrixError, "preprocessed source changed"):
                matrix.run_matrix(self.root)

    def test_preexisting_probe_folder_is_refused_as_potential_stale_result(self):
        descriptor = matrix.descriptor_plan(self.root)[0]
        unique = f"s62_00_{descriptor['source'].replace('/', '_').replace('.c', '')}_O0_{descriptor['source_sha256'][:10]}"
        folder = self.root / "build/probes" / unique
        folder.mkdir(parents=True)
        (folder / "comparison.json").write_text("{}")
        with mock.patch.object(matrix.subprocess, "run") as run:
            with self.assertRaisesRegex(matrix.MatrixError, "refusing stale output"):
                matrix.run_matrix(self.root)
        run.assert_not_called()


if __name__ == "__main__":
    unittest.main()
