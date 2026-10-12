"""Host checks for the nonmatching tree collection wrappers."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class TreeCollectCandidateTests(unittest.TestCase):
    def test_root_guards_and_callback_state(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'tree-collect-test'
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'tests/c/tree_collect_harness.c'),
                '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
