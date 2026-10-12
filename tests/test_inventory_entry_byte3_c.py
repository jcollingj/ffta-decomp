"""Bounded inventory byte-3 mutation, partition selection and first-match behavior."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class InventoryEntryByte3CTests(unittest.TestCase):
    def test_bounds_all_slots_modes_and_first_match(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'inventory-entry-byte3-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'),
                            str(ROOT / 'tests/c/inventory_entry_byte3_harness.c'), '-o', str(program)],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
