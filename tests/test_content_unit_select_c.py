"""Host checks for the recovered content-unit selection callback."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class ContentUnitSelectTests(unittest.TestCase):
    def test_real_callback_and_unit_helpers(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "content-unit-select-test"
            subprocess.run([
                "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                "-Wno-pointer-to-int-cast", "-I", str(ROOT / "include"),
                str(ROOT / "tests/c/content_unit_select_harness.c"),
                str(ROOT / "src/content_unit_select.c"),
                str(ROOT / "src/unit_fields.c"),
                "-o", str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
