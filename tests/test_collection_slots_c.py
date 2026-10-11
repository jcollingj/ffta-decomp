"""Actual collection routines, including full capacity and holes."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class CollectionSlotsCTests(unittest.TestCase):
    def test_counts_first_hole_duplicates_zero_and_full_collection(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'collection-slots-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'),
                            str(ROOT / 'tests/c/collection_slots_harness.c'), '-o', str(program)],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
