"""Behavioral checks for the nonmatching original tree traversal candidate."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class ListTraversalCandidateTests(unittest.TestCase):
    def test_order_stop_successor_mutation_and_root_guards(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'list-traversal-test'
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'tests/c/list_traversal_harness.c'), '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
