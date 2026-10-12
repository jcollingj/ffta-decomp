"""Focused behavior checks for the unlinked equipment-set candidate."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class EquipmentSetCTests(unittest.TestCase):
    def test_category_legality_counters_support_and_selected_failure(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'equipment-set-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'),
                            str(ROOT / 'tests/c/equipment_set_harness.c'), '-o', str(program)],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
