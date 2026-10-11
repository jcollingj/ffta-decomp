"""Fast behavioral checks for actual recovered inventory, ability, and unit C source."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class InventoryCTests(unittest.TestCase):
    def test_job_mastery_thresholds_and_flag_selection(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'job-ability-progress-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'), str(ROOT / 'tests/c/job_ability_progress_harness.c'),
                            '-o', str(program)], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)

    def test_unit_pool_first_empty_and_full_boundaries(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'unit-pool-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'), str(ROOT / 'tests/c/unit_pool_harness.c'),
                            '-o', str(program)], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)

    def test_paired_ability_state_and_first_matching_slot(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'ability-slots-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'), str(ROOT / 'tests/c/ability_slots_harness.c'),
                            '-o', str(program)], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)

    def test_inventory_boundaries_and_original_arithmetic(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'inventory-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'), str(ROOT / 'tests/c/inventory_harness.c'),
                            '-o', str(program)], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)

    def test_ability_flags_and_item_job_selection(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'ability-state-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'), str(ROOT / 'tests/c/ability_state_harness.c'),
                            '-o', str(program)], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
