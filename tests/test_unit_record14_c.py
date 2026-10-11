"""Actual packed readers with explicit copied-dependency stubs."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class UnitRecord14CTests(unittest.TestCase):
    def test_slots_copy_bounds_zero_sources_and_fallbacks(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'unit-record14-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'),
                            str(ROOT / 'tests/c/unit_record14_harness.c'), '-o', str(program)],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)

    def test_copy_source_bank_modes_and_all_byte_indices(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'record-copy-source-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'),
                            str(ROOT / 'tests/c/record_copy_source_harness.c'), '-o', str(program)],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
