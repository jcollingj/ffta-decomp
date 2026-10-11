"""Actual eligibility source; external property readers are explicit stubs."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class JobItemEligibilityCTests(unittest.TestCase):
    def test_category_masks_special_items_and_property_narrowing(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'job-item-test'
            subprocess.run(['cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                            '-I', str(ROOT / 'include'),
                            str(ROOT / 'tests/c/job_item_eligibility_harness.c'), '-o', str(program)],
                           check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
