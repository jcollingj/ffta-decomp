"""Real selection loops with only the per-unit action stubbed."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class UnitPoolChecksCTests(unittest.TestCase):
    def test_conditions_all_slots_order_and_mutation_during_scan(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'unit-pool-checks-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'),
                            str(ROOT / 'tests/c/unit_pool_checks_harness.c'), '-o', str(program)],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)

    def test_actual_action_field_predicate_flags_and_pool_loop_together(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'unit-pool-action-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-Wno-pointer-to-int-cast', '-I', str(ROOT / 'include'),
                            str(ROOT / 'tests/c/unit_pool_action_harness.c'), '-o', str(program)],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
