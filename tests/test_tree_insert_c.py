"""Host behavioral checks for the bounded tree insertion candidate."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class TreeInsertCandidateTests(unittest.TestCase):
    def test_signed_order_initialization_and_allocator_observation(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "tree-insert-test"
            subprocess.run([
                "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                "-I", str(ROOT / "include"),
                str(ROOT / "tests/c/tree_insert_harness.c"),
                str(ROOT / "src/nonmatching/tree_insert.c"), "-o", str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
