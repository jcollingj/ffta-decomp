"""Focused whole-state checks for the counted content unit loader."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class ContentUnitLoaderCTests(unittest.TestCase):
    def test_counted_loader_and_callback_mutations(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "content-unit-loader-test"
            subprocess.run([
                "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                "-I", str(ROOT / "include"),
                str(ROOT / "tests/c/content_unit_loader_harness.c"),
                str(ROOT / "src/content_unit_loader.c"),
                "-o", str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True,
                           text=True)


if __name__ == "__main__":
    unittest.main()
