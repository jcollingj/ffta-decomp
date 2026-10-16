"""Separate-TU composition checks for the known content-state prefix."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class ContentStateCTests(unittest.TestCase):
    def test_real_content_index_and_value_readers(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'content-state-test'
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'tests/c/content_state_harness.c'),
                str(ROOT / 'src/content_index.c'),
                str(ROOT / 'src/content_values.c'),
                '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True,
                           text=True)
