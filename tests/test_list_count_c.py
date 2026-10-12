"""Host behavior checks for the recovered linked-list counter."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class ListCountCandidateTests(unittest.TestCase):
    def test_count_and_read_only_behavior(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'list-count-test'
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'tests/c/list_count_harness.c'), '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
