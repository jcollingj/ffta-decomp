"""Host checks for the recovered unit equipment property summations."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class UnitEquipmentSumsCTests(unittest.TestCase):
    def test_equipment_reader_forwarder_and_property_sum(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'unit-equipment-sums-test'
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'src/unit_equipment_value16.c'),
                str(ROOT / 'src/nonmatching/unit_equipment_sum.c'),
                str(ROOT / 'tests/c/unit_equipment_sums_harness.c'),
                '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
