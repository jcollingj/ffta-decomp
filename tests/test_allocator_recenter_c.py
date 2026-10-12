"""Host behavior checks for the isolated allocator recenter candidate."""

from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


class AllocatorRecenterTests(unittest.TestCase):
    def test_packed_buffer_mutation(self):
        with tempfile.TemporaryDirectory() as directory:
            executable = Path(directory) / "allocator-recenter-test"
            subprocess.run([
                "cc", "-std=gnu99", "-Wall", "-Wextra", "-Werror",
                "-I", str(ROOT / "include"),
                str(ROOT / "src/nonmatching/allocator_recenter.c"),
                str(ROOT / "tests/c/allocator_recenter_harness.c"),
                "-o", str(executable),
            ], check=True)
            result = subprocess.run([str(executable)], text=True,
                                    capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("behavior checks passed", result.stdout)


if __name__ == "__main__":
    unittest.main()
