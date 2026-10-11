"""Behavior and packed layout checks; ARM byte matching remains mandatory."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class Record70CTests(unittest.TestCase):
    def test_all_selectors_and_actual_decoder_against_raw_packed_fields(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'record70-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-Wno-pointer-to-int-cast', '-I', str(ROOT / 'include'),
                            str(ROOT / 'tests/c/record70_harness.c'), '-o', str(program)],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
