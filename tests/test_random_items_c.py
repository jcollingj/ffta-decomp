"""Host behavior checks for the random item slot routine."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class RandomItemCTests(unittest.TestCase):
    def test_fixed_rng_vectors_with_retries_and_duplicate_items(self):
        with tempfile.TemporaryDirectory() as directory:
            for name, fixture in [('retry', '1'), ('duplicates', '0')]:
                program = Path(directory) / f'random-items-{name}'
                subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                                '-DRANDOM_ITEMS_RETRY_FIXTURE=' + fixture,
                                '-I', str(ROOT / 'include'),
                                str(ROOT / 'tests/c/random_items_harness.c'), '-o', str(program)],
                               check=True, capture_output=True, text=True)
                subprocess.run([str(program)], check=True, capture_output=True, text=True)
