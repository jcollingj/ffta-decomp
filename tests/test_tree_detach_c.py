"""Host behavior checks for the tree detach/reinsert candidate."""
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


class TreeDetachCandidateTests(unittest.TestCase):
    def test_whole_tree_mutations(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "tree-detach-test"
            subprocess.run([
                "cc", "-std=c99", "-Wall", "-Wextra", "-Werror",
                "-DTREE_DETACH_HOST_TEST", "-I", str(ROOT / "include"),
                str(ROOT / "tests/c/tree_detach_harness.c"),
                str(ROOT / "src/nonmatching/tree_detach.c"), "-o", str(program),
            ], check=True)
            subprocess.run([str(program)], check=True)


if __name__ == "__main__":
    unittest.main()
