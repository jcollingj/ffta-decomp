"""Behavioral checks for the unintegrated collection class reader."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class CollectionClassReaderCTests(unittest.TestCase):
    def test_thresholds_full_u16_fixture_sweep_and_read_only_tables(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'collection-class-reader-test'
            subprocess.run([
                'cc', '-std=c99', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'tests/c/collection_class_reader_harness.c'),
                '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
