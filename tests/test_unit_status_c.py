"""Host behavioral checks supplement the mandatory ARM byte comparison."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class UnitStatusCTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        directory = tempfile.TemporaryDirectory()
        cls.addClassCleanup(directory.cleanup)
        cls.program = Path(directory.name) / 'unit-status-test'
        subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                        '-I', str(ROOT / 'include'), str(ROOT / 'tests/c/unit_status_harness.c'),
                        '-o', str(cls.program)], check=True, capture_output=True, text=True)

    def run_case(self, case):
        subprocess.run([str(self.program), case], check=True, capture_output=True, text=True)

    def test_opposing_counters_cancel_tick_and_preserve_frozen_values(self):
        self.run_case('counters')

    def test_excluded_values_and_blocking_states_preserve_unit(self):
        self.run_case('exclusions')

    def test_indexed_state_preserves_neighbor_bits_and_bytes(self):
        self.run_case('bits')

    def test_all_fixed_accessors_preserve_other_bits(self):
        self.run_case('fixed')

    def test_counter_accessors_preserve_other_bytes(self):
        self.run_case('counter-accessors')

    def test_counter_add_caps_cancels_and_preserves_frozen_units(self):
        self.run_case('counter-add')
