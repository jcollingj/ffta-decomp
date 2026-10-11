"""Uncounted equipment-provider candidate semantics and recovered inspector integration."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class EquipmentProviderCandidateTests(unittest.TestCase):
    def test_candidate_pairing_and_actual_inspector_integration(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'equipment-provider-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror', '-Wno-pointer-to-int-cast',
                            '-I', str(ROOT / 'include'),
                            str(ROOT / 'tests/c/equipment_provider_harness.c'), '-o', str(program)],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
