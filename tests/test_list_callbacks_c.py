"""Behavior checks for the three unintegrated list callback candidates."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class ListCallbackCandidateTests(unittest.TestCase):
    def test_callbacks_and_flag_word_behavior(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'list-callbacks-test'
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'tests/c/list_callbacks_harness.c'), '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
