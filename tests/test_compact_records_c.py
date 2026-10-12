"""Host behavior checks for the compact-records recovery candidate."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class CompactRecordsCTests(unittest.TestCase):
    def test_compaction_order_callbacks_and_guards(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'compact-records-test'
            subprocess.run(['cc', '-std=gnu99', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'),
                            str(ROOT / 'tests/c/compact_records_harness.c'), '-o', str(program)],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
