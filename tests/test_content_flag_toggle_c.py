"""Host behavior checks for the bounded content global flag routine."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class ContentFlagToggleCTests(unittest.TestCase):
    def test_all_byte_values_match_fixed_whole_owner_images(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "content-flag-toggle-test"
            subprocess.run([
                "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                "-I", str(ROOT / "include"),
                str(ROOT / "tests/c/content_flag_toggle_harness.c"),
                str(ROOT / "src/content_flag_toggle.c"),
                str(ROOT / "src/content_globals.c"),
                "-o", str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True,
                           text=True)
