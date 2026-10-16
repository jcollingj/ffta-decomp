"""Host-side packed-state checks for the allocator insertion candidate."""

from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


class AllocatorInsertTests(unittest.TestCase):
    def test_whole_buffer_mutations_and_dependency_order(self):
        with tempfile.TemporaryDirectory() as directory:
            executable = Path(directory) / "allocator-insert-test"
            subprocess.run([
                "cc", "-std=gnu99", "-Wall", "-Wextra", "-Werror",
                "-I", str(ROOT / "include"),
                str(ROOT / "src/nonmatching/allocator_insert.c"),
                str(ROOT / "tests/c/allocator_insert_harness.c"),
                "-o", str(executable),
            ], check=True)
            result = subprocess.run([str(executable)], text=True,
                                    capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("allocator insert whole-buffer checks passed",
                          result.stdout)


if __name__ == "__main__":
    unittest.main()
