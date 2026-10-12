"""Host behavior checks for the linked-list visitor candidate."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class ListVisitCandidateTests(unittest.TestCase):
    def test_iteration_stopping_callback_mutation_and_guards(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'list-visit-test'
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'tests/c/list_visit_harness.c'), '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
