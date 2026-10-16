import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "wrapper_audit", ROOT / "tools/decomp/wrapper_audit.py")
wrapper_audit = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(wrapper_audit)


class WrapperAuditTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        for folder in ("asm", "src", "data", "config"):
            (self.root / folder).mkdir()
        self.regions = {
            "rom_size": 16, "code_inventory_complete": False, "functions": [],
            "regions": [
                {"start": 0, "end": 8, "kind": "copied", "source": "asm/rom_live.s"},
                {"start": 8, "end": 16, "kind": "source_data", "source": "src/data.c"},
            ],
        }
        self.manifest_path = self.root / "config/decomp_regions.json"
        self.manifest_path.write_text(json.dumps(self.regions))
        (self.root / "asm/rom_live.s").write_text(
            '@ copied wrapper\n.section .text\n.incbin "baserom.gba", 0, 8\n')
        (self.root / "src/data.c").write_text("/* source-backed data */\n")
        (self.root / "Makefile").write_text(
            "OBJS := build/rom_live.o build/data.o\n")

    def tree_snapshot(self):
        return {p.relative_to(self.root).as_posix(): p.read_bytes()
                for p in sorted(self.root.rglob("*")) if p.is_file()}

    def test_absent_orphan_is_clean_and_active_wrapper_is_valid_retired_source(self):
        report = wrapper_audit.audit(self.root)
        self.assertTrue(report["ok"])
        self.assertEqual(report["empty_orphans"], [])
        self.assertEqual(report["nonempty_orphans"], [])

    def test_empty_and_nonempty_orphans_are_reported_separately(self):
        (self.root / "asm/rom_empty_old.s").write_text(" \n\t")
        (self.root / "asm/rom_nonempty_old.s").write_text(
            '.section .text\n.incbin "baserom.gba", 8, 4\n')
        # A rom-prefixed authored assembly source is not a copied wrapper.
        (self.root / "asm/rom_authored.s").write_text(".section .text\nmov r0, r1\n")
        before = self.tree_snapshot()
        report = wrapper_audit.audit(self.root)
        after = self.tree_snapshot()
        self.assertEqual(before, after)
        self.assertFalse(report["ok"])
        self.assertEqual(report["empty_orphans"], ["asm/rom_empty_old.s"])
        self.assertEqual(report["nonempty_orphans"], ["asm/rom_nonempty_old.s"])
        self.assertEqual(report["orphan_count"], 2)

    def test_ledger_wrapper_without_active_object_is_reported_as_mismatch(self):
        (self.root / "Makefile").write_text("OBJS := build/data.o\n")
        report = wrapper_audit.audit(self.root)
        self.assertFalse(report["ok"])
        self.assertEqual(report["missing_active_sources"], [{
            "source": "asm/rom_live.s",
            "reason": "copied ledger source is missing from active OBJS",
        }])
        self.assertEqual(report["orphan_count"], 0)

    def test_active_wrapper_missing_from_ledger_is_reported(self):
        self.regions["regions"][0]["kind"] = "source_data"
        self.manifest_path.write_text(json.dumps(self.regions))
        report = wrapper_audit.audit(self.root)
        self.assertFalse(report["ok"])
        self.assertEqual(report["active_wrappers_missing_ledger"], ["asm/rom_live.s"])
        self.assertEqual(report["orphan_count"], 0)

    def test_missing_makefile_active_source_is_explicit_and_no_orphans_inferred(self):
        (self.root / "Makefile").write_text("OBJS := build/rom_live.o build/missing.o\n")
        report = wrapper_audit.audit(self.root)
        self.assertFalse(report["ok"])
        self.assertEqual(report["missing_active_sources"][0]["object"], "build/missing.o")
        self.assertEqual(report["orphan_count"], 0)

    def test_missing_active_wrapper_file_is_reported_without_orphan_verdict(self):
        (self.root / "asm/rom_live.s").unlink()
        before = self.tree_snapshot()
        report = wrapper_audit.audit(self.root)
        self.assertEqual(before, self.tree_snapshot())
        self.assertFalse(report["ok"])
        self.assertEqual(report["missing_active_sources"][0]["object"], "build/rom_live.o")
        self.assertEqual(report["orphan_count"], 0)

    def test_malformed_incbin_fails_closed(self):
        (self.root / "asm/rom_suspicious.s").write_text(
            '.section .text\n.incbin "other.gba", 8, 4\n')
        before = self.tree_snapshot()
        with self.assertRaisesRegex(ValueError, "unrecognized ROM wrapper incbin"):
            wrapper_audit.audit(self.root)
        self.assertEqual(before, self.tree_snapshot())

    def test_corrupt_whole_ledger_fails_closed(self):
        self.regions["regions"][1]["start"] = 9
        self.manifest_path.write_text(json.dumps(self.regions))
        (self.root / "asm/rom_empty_old.s").write_text("")
        with self.assertRaises(ValueError):
            wrapper_audit.audit(self.root)

    def test_cli_json_success_and_error_exit_are_machine_readable(self):
        command = [sys.executable, str(ROOT / "tools/decomp/wrapper_audit.py"),
                   "--root", str(self.root), "--json"]
        success = subprocess.run(command, text=True, capture_output=True, check=False)
        self.assertEqual(success.returncode, 0, success.stderr)
        self.assertTrue(json.loads(success.stdout)["ok"])

        (self.root / "asm/rom_old.s").write_text("")
        before = self.tree_snapshot()
        orphan = subprocess.run(command, text=True, capture_output=True, check=False)
        self.assertEqual(orphan.returncode, 1, orphan.stderr)
        parsed = json.loads(orphan.stdout)
        self.assertEqual(parsed["empty_orphans"], ["asm/rom_old.s"])
        self.assertEqual(before, self.tree_snapshot())

        self.regions["regions"][1]["start"] = 9
        self.manifest_path.write_text(json.dumps(self.regions))
        invalid = subprocess.run(command, text=True, capture_output=True, check=False)
        self.assertEqual(invalid.returncode, 2)
        self.assertTrue(json.loads(invalid.stdout)["errors"])
        self.assertEqual(json.loads(invalid.stdout)["orphan_count"], 0)

    def test_cli_text_reports_actionable_malformed_wrapper_error(self):
        (self.root / "asm/rom_suspicious.s").write_text(
            '.section .text\n.incbin "other.gba", 8, 4\n')
        command = [sys.executable, str(ROOT / "tools/decomp/wrapper_audit.py"),
                   "--root", str(self.root)]
        before = self.tree_snapshot()
        result = subprocess.run(command, text=True, capture_output=True, check=False)
        self.assertEqual(result.returncode, 2)
        self.assertIn("Wrapper audit failed:", result.stderr)
        self.assertIn("unrecognized ROM wrapper incbin in asm/rom_suspicious.s", result.stderr)
        self.assertEqual(before, self.tree_snapshot())


if __name__ == "__main__":
    unittest.main()
