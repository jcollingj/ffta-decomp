"""Host behavior checks for the list search callback."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class ListSearchCallbackCandidateTests(unittest.TestCase):
    def test_observed_search_and_store_behavior(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'list-search-callback-test'
            subprocess.run([
                'cc', '-std=gnu11', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'tests/c/list_search_callback_harness.c'),
                str(ROOT / 'src/list_search_callback.c'),
                str(ROOT / 'src/list_accessors.c'), '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
