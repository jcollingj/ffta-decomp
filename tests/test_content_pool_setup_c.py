"""Separate-translation-unit whole-state checks for content-pool setup."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class ContentPoolSetupCTests(unittest.TestCase):
    def test_loader_then_occupied_entry_preparation(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "content-pool-setup-test"
            subprocess.run([
                "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                "-I", str(ROOT / "include"),
                str(ROOT / "tests/c/content_pool_setup_harness.c"),
                str(ROOT / "src/content_pool_setup.c"),
                "-o", str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True,
                           text=True)


if __name__ == "__main__":
    unittest.main()
