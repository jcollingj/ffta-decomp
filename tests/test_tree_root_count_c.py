"""Behavioral and read-only checks for the direct-root tree count candidate."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class TreeRootCountCandidateTests(unittest.TestCase):
    def test_tree_shapes_counts_subroots_and_preserves_state(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'tree-root-count-test'
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'tests/c/tree_root_count_harness.c'), '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
