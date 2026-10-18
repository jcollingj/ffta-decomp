"""Original bounded pointer search and real encode/decode composition."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires host C compiler")
class UnitReferenceEncodeTests(unittest.TestCase):
    def test_search_bounds_first_match_and_real_decode(self):
        with tempfile.TemporaryDirectory() as temp:
            exe = Path(temp) / "unit-reference-encode"
            subprocess.run([
                "cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                "-I", str(ROOT / "include"),
                str(ROOT / "src/unit_reference_encode.c"),
                str(ROOT / "src/unit_reference.c"),
                str(ROOT / "tests/c/unit_reference_encode_harness.c"),
                "-o", str(exe),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(exe)], check=True, capture_output=True, text=True)
