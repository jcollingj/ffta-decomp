"""Host mutation checks for the provisional direct equipment replacement routine."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class EquipmentReplaceDirectCTests(unittest.TestCase):
    def test_ability_cleanup_item_swap_and_empty_slot(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'equipment-replace-direct-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'),
                            str(ROOT / 'tests/c/equipment_replace_direct_harness.c'),
                            '-o', str(program)], check=True, capture_output=True, text=True)
            result = subprocess.run([str(program)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
