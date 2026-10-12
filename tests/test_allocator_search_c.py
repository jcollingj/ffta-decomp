"""Host behavior checks for the isolated allocator search candidate."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class AllocatorSearchCandidateTests(unittest.TestCase):
    def test_literal_linked_size_search_and_output_stores(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'allocator-search-test'
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'src/nonmatching/allocator_search.c'),
                str(ROOT / 'tests/c/allocator_search_harness.c'),
                '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
