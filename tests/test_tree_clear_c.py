"""Behavioral checks for the postorder tree-clear candidate."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class TreeClearCandidateTests(unittest.TestCase):
    def test_fixed_postorder_shapes_and_caller_state(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'tree-clear-test'
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'tests/c/tree_clear_harness.c'),
                str(ROOT / 'src/tree_clear.c'), '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
