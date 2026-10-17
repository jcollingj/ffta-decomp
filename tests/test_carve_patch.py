import importlib.util
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("carve_patch", ROOT / "tools/decomp/carve_patch.py")
carve_patch = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(carve_patch)


class CarvePatchTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name) / "fixture"
        for folder in ("asm", "src", "data", "config"):
            (self.root / folder).mkdir(parents=True, exist_ok=True)
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
        (self.root / "asm/rom_b.s").write_text(
            '@ old wrapper comment\n.section .text\n.incbin "baserom.gba", 0x180, 0xE80\n')
        (self.root / "src/old.c").write_text(
            "void ExistingFixtureFunction(void) {}\n")
        (self.root / "src/first.c").write_text("")
        (self.root / "src/last.c").write_text("")
        (self.root / "Makefile").write_text(
            "PRELUDE := retained # keep inline note\nOBJS    := \\\n"
            "\tbuild/first.o \\\n"
            "\tbuild/rom_b.o \\\n"
            "\tbuild/last.o\nPOSTLUDE := retained\n")

    def tearDown(self):
        self.temp.cleanup()

    def _snapshot(self, root):
        return {path.relative_to(root).as_posix(): path.read_bytes()
                for path in root.rglob("*") if path.is_file()}

    def _apply(self, patch):
        copy = Path(self.temp.name) / f"apply-{len(list(Path(self.temp.name).glob('apply-*')))}"
        shutil.copytree(self.root, copy)
        patch_path = Path(self.temp.name) / "carve.patch"
        patch_path.write_text(patch)
        subprocess.run(["git", "apply", "--check", str(patch_path)], cwd=copy,
                       check=True, capture_output=True, text=True)
        subprocess.run(["git", "apply", str(patch_path)], cwd=copy,
                       check=True, capture_output=True, text=True)
        return copy

    def test_all_carve_geometries_apply_exact_wrapper_and_object_changes(self):
        cases = [
            (0x180, 0x1A0, [("asm/rom_b_suffix.s", 0x1A0, 0xE60)]),
            (0xFE0, 0x1000, [("asm/rom_b_prefix.s", 0x180, 0xE60)]),
            (0x200, 0x240, [("asm/rom_b_prefix.s", 0x180, 0x80),
                            ("asm/rom_b_suffix.s", 0x240, 0xDC0)]),
            (0x180, 0x1000, []),
        ]
        for number, (start, end, remainders) in enumerate(cases):
            with self.subTest(start=start, end=end):
                before = self._snapshot(self.root)
                patch = carve_patch.render(self.root, start, end, f"src/new_{number}.c")
                applied = self._apply(patch)
                self.assertFalse((applied / "asm/rom_b.s").exists())
                makefile = (applied / "Makefile").read_text()
                self.assertIn("PRELUDE := retained # keep inline note\n", makefile)
                self.assertIn("POSTLUDE := retained\n", makefile)
                self.assertLess(makefile.index("build/first.o"), makefile.index(f"build/new_{number}.o"))
                self.assertLess(makefile.index(f"build/new_{number}.o"), makefile.index("build/last.o"))
                self.assertEqual([p for p, _, _ in remainders],
                                 [p for p in ("asm/rom_b_prefix.s", "asm/rom_b_suffix.s")
                                  if (applied / p).exists()])
                for path, offset, length in remainders:
                    content = (applied / path).read_text()
                    self.assertIn(f'.incbin "baserom.gba", {offset:#x}, {length:#x}', content)
                self.assertEqual(self._snapshot(self.root), before)

    def test_unrelated_objects_and_comment_order_are_preserved(self):
        patch = carve_patch.render(self.root, 0x200, 0x240, "src/new_code.c")
        applied = self._apply(patch)
        makefile = (applied / "Makefile").read_text()
        self.assertEqual([line.strip().split()[0] for line in makefile.splitlines()
                         if line.strip().startswith("build/")], [
                             "build/first.o", "build/rom_b_prefix.o", "build/new_code.o",
                             "build/rom_b_suffix.o", "build/last.o"])
        self.assertIn("PRELUDE := retained # keep inline note", makefile)

    def test_invalid_paths_and_collisions_reuse_planner_rejection(self):
        for kwargs, message in [
            ({"source": "src/../bad.c"}, "invalid new"),
            ({"source": "src/new_code.c", "prefix": "src/wrong.c"}, "invalid prefix"),
        ]:
            with self.subTest(kwargs=kwargs), self.assertRaisesRegex(ValueError, message):
                carve_patch.render(self.root, 0x200, 0x240, **kwargs)
        (self.root / "asm/rom_b_prefix.s").write_text("")
        with self.assertRaisesRegex(ValueError, "already exists"):
            carve_patch.render(self.root, 0x200, 0x240, "src/new_code.c")

    def test_unrecognized_inline_object_layout_fails_without_losing_comment(self):
        (self.root / "Makefile").write_text("OBJS := build/rom_b.o build/last.o\n")
        with self.assertRaisesRegex(ValueError, "alone on its line"):
            carve_patch.render(self.root, 0x200, 0x240, "src/new_code.c")
        self.assertIn("build/rom_b.o build/last.o", (self.root / "Makefile").read_text())

    def test_planner_rejects_comments_inside_objs_assignment(self):
        (self.root / "Makefile").write_text(
            "OBJS := \\\n\tbuild/first.o \\\n\tbuild/rom_b.o \\ # inline note\n\tbuild/last.o\n")
        with self.assertRaisesRegex(ValueError, "unsupported OBJS token"):
            carve_patch.render(self.root, 0x200, 0x240, "src/new_code.c")

    def test_applied_patch_handles_old_wrapper_without_final_newline(self):
        wrapper = self.root / "asm/rom_b.s"
        wrapper.write_bytes(wrapper.read_bytes().rstrip(b"\n"))
        before = self._snapshot(self.root)
        patch = carve_patch.render(self.root, 0x200, 0x240, "src/new_code.c")
        self.assertIn("\\ No newline at end of file\n", patch)
        applied = self._apply(patch)
        self.assertFalse((applied / "asm/rom_b.s").exists())
        self.assertEqual(self._snapshot(self.root), before)

    def test_eof_marker_tracks_line_position_for_repeated_wrapper_lines(self):
        wrapper = self.root / "asm/rom_b.s"
        wrapper.write_bytes(wrapper.read_bytes() + b"@ repeated\n@ repeated")
        before = self._snapshot(self.root)
        patch = carve_patch.render(self.root, 0x200, 0x240, "src/new_code.c")
        repeated_deletion = "-@ repeated\n-@ repeated\n\\ No newline at end of file\n"
        self.assertIn(repeated_deletion, patch)
        self.assertEqual(patch.count("\\ No newline at end of file\n"), 1)
        applied = self._apply(patch)
        self.assertFalse((applied / "asm/rom_b.s").exists())
        self.assertEqual(self._snapshot(self.root), before)

    def test_applied_patch_preserves_makefile_without_final_newline(self):
        makefile = self.root / "Makefile"
        makefile.write_bytes(
            b"PRELUDE := retained\nOBJS := \\\n"
            b"\tbuild/first.o \\\n"
            b"\tbuild/rom_b.o")
        before = self._snapshot(self.root)
        patch = carve_patch.render(self.root, 0x200, 0x240, "src/new_code.c")
        self.assertIn("\\ No newline at end of file\n", patch)
        applied = self._apply(patch)
        after = (applied / "Makefile").read_bytes()
        self.assertFalse(after.endswith(b"\n"))
        text = after.decode()
        for obj in ("build/first.o", "build/rom_b_prefix.o", "build/new_code.o",
                    "build/rom_b_suffix.o"):
            self.assertIn(obj, text)
        self.assertEqual(self._snapshot(self.root), before)

    def test_cli_emits_git_apply_patch_without_mutating_fixture(self):
        before = self._snapshot(self.root)
        command = ["python3", str(ROOT / "tools/decomp/carve_patch.py"),
                   "--root", str(self.root), "--start", "0x200", "--end", "0x240",
                   "--source", "src/new_code.c"]
        result = subprocess.run(command, cwd=self.root, check=True, capture_output=True, text=True)
        self._apply(result.stdout)
        self.assertEqual(self._snapshot(self.root), before)


if __name__ == "__main__":
    unittest.main()
