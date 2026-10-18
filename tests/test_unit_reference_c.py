"""Native-pointer and unaligned index tests for the reference resolvers."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires host C compiler")
class UnitReferenceTests(unittest.TestCase):
    def test_table_selection_fallback_and_unaligned_indices(self):
        with tempfile.TemporaryDirectory() as temp:
            exe = Path(temp) / "unit-reference"
            subprocess.run([
                "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                "-I", str(ROOT / "include"),
                str(ROOT / "src/unit_reference.c"),
                str(ROOT / "tests/c/unit_reference_harness.c"),
                "-o", str(exe),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(exe)], check=True, capture_output=True, text=True)
