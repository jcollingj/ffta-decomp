"""Host behavior checks for the allocator wrapper candidates."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class AllocatorWrapperCandidateTests(unittest.TestCase):
    def test_callbacks_rounding_narrowing_and_header_read(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'allocator-wrappers-test'
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'src/allocator_wrappers.c'),
                str(ROOT / 'tests/c/allocator_wrappers_harness.c'),
                '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
