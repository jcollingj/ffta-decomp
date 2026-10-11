"""Recovered RNG fixed vectors, state replay, and five-slot count."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class RandomCTests(unittest.TestCase):
    def test_vectors_reseed_pointer_and_all_slot_patterns(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'random-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'),
                            str(ROOT / 'tests/c/random_harness.c'), '-o', str(program)],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
