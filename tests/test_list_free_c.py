"""Host behavior checks for the recursive list free candidate."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class ListFreeCandidateTests(unittest.TestCase):
    def test_null_single_multiple_and_captured_unwind_path(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "list-free-test"
            subprocess.run([
                "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                "-I", str(ROOT / "include"),
                str(ROOT / "tests/c/list_free_harness.c"),
                str(ROOT / "src/list_free.c"),
                "-o", str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
