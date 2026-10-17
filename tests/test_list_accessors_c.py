"""Host behavior checks for the uncounted list accessor candidates."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class ListAccessorCandidateTests(unittest.TestCase):
    def test_accessors_and_copy_behavior(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'list-accessors-test'
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'tests/c/list_accessors_harness.c'),
                str(ROOT / 'src/list_accessors.c'), '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
