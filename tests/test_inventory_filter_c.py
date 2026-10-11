"""Actual inventory partition, class, copy and search integration."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class InventoryFilterCTests(unittest.TestCase):
    def test_full_ranges_classification_and_inventory_selection(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'inventory-filter-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'),
                            str(ROOT / 'tests/c/inventory_filter_harness.c'), '-o', str(program)],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
