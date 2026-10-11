"""Real scan control flow with explicit inspection/replacement stubs."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class EquipmentScanCTests(unittest.TestCase):
    def test_masks_slots_clear_modes_and_restart_order(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'equipment-scan-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'),
                            str(ROOT / 'tests/c/equipment_scan_harness.c'), '-o', str(program)],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
