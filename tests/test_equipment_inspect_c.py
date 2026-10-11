"""Equipment removal/restoration and ability-loss flags with dependency assertions."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class EquipmentInspectCTests(unittest.TestCase):
    def test_temporary_removal_restoration_and_ability_flags(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'equipment-inspect-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'),
                            str(ROOT / 'tests/c/equipment_inspect_harness.c'), '-o', str(program)],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
