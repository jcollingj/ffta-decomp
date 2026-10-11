"""Real amount/flag routines; only adjustment predicate is stubbed."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class ItemAmountCTests(unittest.TestCase):
    def test_wrap_signed_amount_floors_and_flag_updates(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'item-amount-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'),
                            str(ROOT / 'tests/c/item_amount_harness.c'), '-o', str(program)],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)

    def test_actual_adjustment_lookup_property_reader_and_amount_together(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'item-adjustment-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'),
                            str(ROOT / 'tests/c/item_adjustment_harness.c'), '-o', str(program)],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
