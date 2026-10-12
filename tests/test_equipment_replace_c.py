"""Behavioral checks for the uncounted ReplaceEquipment candidate."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class EquipmentReplaceCandidateTests(unittest.TestCase):
    def test_replacement_side_effects_and_guards(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "equipment-replace-test"
            subprocess.run(
                ["cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                 "-Wno-pointer-to-int-cast", "-I", str(ROOT / "include"),
                 str(ROOT / "tests/c/equipment_replace_harness.c"),
                 str(ROOT / "src/ability_slots.c"), "-o", str(program)],
                check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
