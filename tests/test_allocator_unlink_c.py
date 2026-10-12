"""Host-side state checks for the allocator unlink candidate."""

from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


class AllocatorUnlinkTests(unittest.TestCase):
    def test_whole_packed_buffer_and_recenter_call_order(self):
        with tempfile.TemporaryDirectory() as directory:
            executable = Path(directory) / "allocator-unlink-test"
            subprocess.run([
                "cc", "-std=gnu99", "-Wall", "-Wextra", "-Werror",
                "-I", str(ROOT / "include"),
                str(ROOT / "src/nonmatching/allocator_unlink.c"),
                str(ROOT / "tests/c/allocator_unlink_harness.c"),
                "-o", str(executable),
            ], check=True)
            result = subprocess.run([str(executable)], check=True, text=True,
                                    capture_output=True)
            self.assertIn("allocator unlink state checks passed", result.stdout)


if __name__ == "__main__":
    unittest.main()
