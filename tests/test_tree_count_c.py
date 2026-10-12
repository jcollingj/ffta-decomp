"""Behavioral and read-only checks for the tree traversal candidate."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class TreeCountCandidateTests(unittest.TestCase):
    def test_tree_shapes_count_and_preserve_all_state(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'tree-count-test'
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'tests/c/tree_count_harness.c'), '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
