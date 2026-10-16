"""Host behavior checks for the linked-list append candidate."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class ListAppendCandidateTests(unittest.TestCase):
    def test_owner_and_node_images_for_append_cases(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "list-append-test"
            subprocess.run([
                "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                "-I", str(ROOT / "include"),
                str(ROOT / "tests/c/list_append_harness.c"),
                str(ROOT / "src/list_append.c"),
                "-o", str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True,
                           capture_output=True, text=True)
