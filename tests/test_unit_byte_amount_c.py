"""Exhaustive host check for recovered unit-byte amount calculation."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class UnitByteAmountCTests(unittest.TestCase):
    def test_all_byte_pairs_and_read_only_unit(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'unit-byte-amount-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'),
                            str(ROOT / 'tests/c/unit_byte_amount_harness.c'), '-o', str(program)],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
