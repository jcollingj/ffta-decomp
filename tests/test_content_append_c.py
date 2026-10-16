"""Behavior checks for the bounded content output append callback."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class ContentAppendTests(unittest.TestCase):
    def test_callback_native_pointer_and_wrap(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'content-append-test'
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'tests/c/content_append_harness.c'),
                str(ROOT / 'src/content_append.c'),
                '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
