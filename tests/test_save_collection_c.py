"""Actual partial save routines, layout, stride and counter boundaries."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class SaveCollectionCTests(unittest.TestCase):
    def test_record_presence_count_and_counter_preserve_other_save_bytes(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'save-collection-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'),
                            str(ROOT / 'tests/c/save_collection_harness.c'), '-o', str(program)],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
