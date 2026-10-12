"""Behavioral checks for the matching reverse tree visitor."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class TreeReverseVisitCandidateTests(unittest.TestCase):
    def test_order_stop_mutation_subtree_and_immutability(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'tree-reverse-visit-test'
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'tests/c/tree_reverse_visit_harness.c'), '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
