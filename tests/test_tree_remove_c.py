"""Host checks for the tree detach-and-free wrapper."""
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


class TreeRemoveWrapperTests(unittest.TestCase):
    def test_detach_mutation_and_reloaded_allocator(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "tree-remove-test"
            subprocess.run([
                "cc", "-std=c99", "-Wall", "-Wextra", "-Werror",
                "-DTREE_REMOVE_HOST_TEST", "-I", str(ROOT / "include"),
                str(ROOT / "tests/c/tree_remove_harness.c"),
                str(ROOT / "src/tree_remove.c"),
                "-o", str(program),
            ], check=True)
            subprocess.run([str(program)], check=True)


if __name__ == "__main__":
    unittest.main()
