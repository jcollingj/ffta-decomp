"""Host checks for the coordinate-select list callback."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class ContentCoordinateSelectTests(unittest.TestCase):
    def test_real_callback_getter_and_visitor(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "content-coordinate-select-test"
            subprocess.run([
                "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                "-I", str(ROOT / "include"),
                str(ROOT / "tests/c/content_coordinate_select_harness.c"),
                str(ROOT / "src/content_coordinate_select.c"),
                str(ROOT / "src/list_accessors.c"),
                str(ROOT / "src/list_visit.c"),
                "-o", str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True,
                           text=True)
