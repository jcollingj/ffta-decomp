"""Actual mapped-job helpers and inherited property reader together."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class MappedJobCTests(unittest.TestCase):
    def test_mapping_inheritance_narrowing_and_content_lookup(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'mapped-job-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'),
                            str(ROOT / 'tests/c/mapped_job_harness.c'), '-o', str(program)],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
