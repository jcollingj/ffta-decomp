"""Host composition checks for the recovered tree search wrapper."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class TreeSearchCompositionTests(unittest.TestCase):
    def test_real_wrapper_callback_traversal_and_accessor_compose(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'tree-search-test'
            subprocess.run([
                'cc', '-std=gnu11', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'tests/c/tree_search_harness.c'),
                str(ROOT / 'src/tree_search.c'),
                str(ROOT / 'src/list_search_callback.c'),
                str(ROOT / 'src/list_traversal.c'),
                str(ROOT / 'src/list_accessors.c'),
                '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
