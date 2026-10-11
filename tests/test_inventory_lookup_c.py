"""Inventory query ranges, byte wrapping, mapping and mutation integration."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class InventoryLookupCTests(unittest.TestCase):
    def test_ranges_wrapping_duplicates_and_actual_mutations(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'inventory-lookup-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'),
                            str(ROOT / 'tests/c/inventory_lookup_harness.c'), '-o', str(program)],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
