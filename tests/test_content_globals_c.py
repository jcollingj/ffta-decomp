"""Host checks for two read-only native-pointer global getters."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class ContentGlobalsCTests(unittest.TestCase):
    def test_getters_return_pointer_sentinels_without_mutating_inputs(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'content-globals-test'
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'tests/c/content_globals_harness.c'),
                str(ROOT / 'src/content_globals.c'),
                '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True,
                           text=True)
