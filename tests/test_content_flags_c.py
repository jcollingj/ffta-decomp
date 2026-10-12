"""Behavior checks for the recovered content flag routines and clear candidate."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("cc"), "requires a host C compiler")
class ContentFlagCandidateTests(unittest.TestCase):
    def test_flag_copy_order_and_collateral_state(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / "content-flags-test"
            subprocess.run(
                ["cc", "-std=gnu89", "-Wall", "-Wextra", "-Werror",
                 "-I", str(ROOT / "include"),
                 str(ROOT / "tests/c/content_flags_harness.c"), "-o", str(program)],
                check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
