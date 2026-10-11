"""Actual job inheritance, unit/item readers, and equipment mask integration."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class UnitEquipmentMaskCTests(unittest.TestCase):
    def test_inheritance_category_bits_and_mode_zero_wrapper(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'unit-equipment-mask-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'),
                            str(ROOT / 'tests/c/unit_equipment_mask_harness.c'), '-o', str(program)],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
