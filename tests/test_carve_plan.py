import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("carve_plan", ROOT / "tools/decomp/carve_plan.py")
carve_plan = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(carve_plan)


class CarvePlanTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        for folder in ("asm", "src", "data", "config"):
            (self.root / folder).mkdir()
        self.regions = {
            "rom_size": 0x1000,
            "code_inventory_complete": False,
            "functions": [{
                "name": "ExistingFixtureFunction", "start": 0x100, "end": 0x104,
                "status": "matching_c", "source": "src/old.c",
            }],
            "regions": [
                {"start": 0, "end": 0x100, "kind": "copied", "source": "asm/rom_a.s"},
                {"start": 0x100, "end": 0x180, "kind": "decompiled_c", "source": "src/old.c"},
                {"start": 0x180, "end": 0x1000, "kind": "copied", "source": "asm/rom_b.s"},
            ],
        }
        (self.root / "config/decomp_regions.json").write_text(json.dumps(self.regions))
        (self.root / "asm/rom_a.s").write_text('.incbin "baserom.gba", 0x0, 0x100\n')
        (self.root / "asm/rom_b.s").write_text('.incbin "baserom.gba", 0x180, 0xE80\n')
        (self.root / "src/old.c").write_text(
            "void ExistingFixtureFunction(void) {}\n")
        (self.root / "Makefile").write_text(
            "OBJS    := \\\n\tbuild/first.o \\\n\tbuild/rom_b.o \\\n\tbuild/last.o\n")
        (self.root / "src/first.c").write_text("")
        (self.root / "src/last.c").write_text("")

    def tearDown(self):
        self.temp.cleanup()

    def invoke(self, start, end, **kwargs):
        return carve_plan.plan(self.root, start, end, "src/new_code.c", **kwargs)

    def test_beginning_carve_has_only_suffix(self):
        result = self.invoke(0x180, 0x1A0)
        self.assertEqual(result["remainders"], [{
            "side": "suffix", "start": 0x1A0, "end": 0x1000,
            "source": "asm/rom_b_suffix.s", "object": "build/rom_b_suffix.o",
            "incbin": {"offset": 0x1A0, "length": 0xE60},
        }])
        self.assertEqual(result["objects"]["after"], [
            "build/first.o", "build/new_code.o", "build/rom_b_suffix.o", "build/last.o"])

    def test_end_carve_has_only_prefix(self):
        result = self.invoke(0xFE0, 0x1000)
        self.assertEqual(len(result["remainders"]), 1)
        self.assertEqual(result["remainders"][0]["incbin"], {"offset": 0x180, "length": 0xE60})
        self.assertEqual(result["remainders"][0]["side"], "prefix")

    def test_interior_carve_has_both_remainders_in_layout_order(self):
        result = self.invoke(0x200, 0x240)
        self.assertEqual([r["incbin"] for r in result["remainders"]], [
            {"offset": 0x180, "length": 0x80}, {"offset": 0x240, "length": 0xDC0}])
        self.assertEqual(result["objects"]["after"], [
            "build/first.o", "build/rom_b_prefix.o", "build/new_code.o",
            "build/rom_b_suffix.o", "build/last.o"])

    def test_whole_region_carve_has_no_zero_length_remainder(self):
        result = self.invoke(0x180, 0x1000)
        self.assertEqual(result["remainders"], [])
        self.assertEqual(result["objects"]["after"], [
            "build/first.o", "build/new_code.o", "build/last.o"])

    def test_unrelated_objects_are_preserved_in_order(self):
        result = self.invoke(0x200, 0x240)
        self.assertEqual(result["objects"]["unrelated_preserved_in_order"],
                         ["build/first.o", "build/last.o"])

    def test_rejects_empty_outside_unaligned_and_cross_region_intervals(self):
        for start, end in ((0x200, 0x200), (0x1000, 0x1004), (0x181, 0x1A0),
                           (0xF0, 0x110), (0x100, 0x104)):
            with self.subTest(start=start, end=end), self.assertRaises(ValueError):
                self.invoke(start, end)

    def test_rejects_invalid_full_ledger_partition(self):
        for mutation in ("gap", "overlap"):
            with self.subTest(mutation=mutation):
                original = json.loads((self.root / "config/decomp_regions.json").read_text())
                if mutation == "gap":
                    original["regions"][1]["start"] += 4
                else:
                    original["regions"][0]["end"] += 4
                (self.root / "config/decomp_regions.json").write_text(json.dumps(original))
                with self.assertRaises(ValueError):
                    self.invoke(0x200, 0x240)
                (self.root / "config/decomp_regions.json").write_text(json.dumps(self.regions))

    def test_rejects_existing_new_source_collision(self):
        (self.root / "src/new_code.c").write_text("")
        with self.assertRaisesRegex(ValueError, "already exists"):
            self.invoke(0x200, 0x240)

    def test_rejects_existing_remainder_source_collision(self):
        (self.root / "asm/rom_b_prefix.s").write_text("")
        with self.assertRaisesRegex(ValueError, "already exists"):
            self.invoke(0x200, 0x240)

    def test_rejects_object_collision(self):
        (self.root / "asm/new_code.s").write_text("")
        with self.assertRaisesRegex(ValueError, "collides"):
            self.invoke(0x180, 0x1000)

    def test_rejects_invalid_paths_and_unused_named_remainder(self):
        with self.assertRaisesRegex(ValueError, "invalid new"):
            carve_plan.plan(self.root, 0x180, 0x1000, "src/../new.c")
        with self.assertRaisesRegex(ValueError, "no prefix remainder"):
            self.invoke(0x180, 0x200, prefix="asm/custom_prefix.s")
        with self.assertRaisesRegex(ValueError, "invalid prefix"):
            self.invoke(0x200, 0x240, prefix="src/custom_prefix.c")
        with self.assertRaisesRegex(ValueError, "invalid suffix"):
            self.invoke(0x200, 0x240, suffix="asm/./custom_suffix.s")

    def test_rejects_old_source_path_that_escapes_root(self):
        manifest = json.loads((self.root / "config/decomp_regions.json").read_text())
        manifest["regions"][2]["source"] = "../outside.s"
        (self.root / "config/decomp_regions.json").write_text(json.dumps(manifest))
        with self.assertRaisesRegex(ValueError, "invalid old copied"):
            self.invoke(0x200, 0x240)

    def test_rejects_malformed_incbin_and_missing_object(self):
        (self.root / "asm/rom_b.s").write_text('.incbin "baserom.gba", 0x180, ???\n')
        with self.assertRaisesRegex(ValueError, "unrecognized incbin"):
            self.invoke(0x200, 0x240)
        (self.root / "asm/rom_b.s").write_text('.incbin "baserom.gba", 0x180, 0xE80\n')
        (self.root / "Makefile").write_text("OBJS := build/first.o build/last.o\n")
        with self.assertRaisesRegex(ValueError, "exactly once"):
            self.invoke(0x200, 0x240)

    def test_json_representation_is_deterministic_and_read_only(self):
        before = {p.relative_to(self.root).as_posix(): p.read_bytes()
                  for p in self.root.rglob("*") if p.is_file()}
        first = self.invoke(0x200, 0x240)
        second = self.invoke(0x200, 0x240)
        self.assertEqual(json.dumps(first, sort_keys=True, indent=2),
                         json.dumps(second, sort_keys=True, indent=2))
        self.assertTrue(first["read_only"])
        self.assertFalse(first["matching_c_claim"])
        self.assertEqual(first["files"]["edit"], [
            "Makefile", "config/decomp_regions.json", "ld/rom.ld"])
        self.assertEqual(first["files"]["retire"][0]["path"], "asm/rom_b.s")
        after = {p.relative_to(self.root).as_posix(): p.read_bytes()
                 for p in self.root.rglob("*") if p.is_file()}
        self.assertEqual(after, before)

    def test_cli_json_and_invalid_interval_exit_status(self):
        command = [sys.executable, str(ROOT / "tools/decomp/carve_plan.py"),
                   "--root", str(self.root), "--start", "0x200", "--end", "0x240",
                   "--source", "src/new_code.c", "--json"]
        completed = subprocess.run(command, check=True, capture_output=True, text=True)
        decoded = json.loads(completed.stdout)
        self.assertEqual(decoded["carve"]["source"], "src/new_code.c")
        bad = command.copy()
        bad[bad.index("0x240")] = "0x241"
        rejected = subprocess.run(bad, capture_output=True, text=True)
        self.assertNotEqual(rejected.returncode, 0)
        self.assertIn("aligned", rejected.stderr)


if __name__ == "__main__":
    unittest.main()
