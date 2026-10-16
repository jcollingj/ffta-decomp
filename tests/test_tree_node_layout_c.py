"""Host integration checks for the recovered tree node and callbacks."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class TreeNodeLayoutTests(unittest.TestCase):
    def test_layout_and_traversal_callbacks(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'tree-node-layout-test'
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'tests/c/tree_node_layout_harness.c'),
                str(ROOT / 'src/list_callbacks.c'),
                str(ROOT / 'src/list_traversal.c'), '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
