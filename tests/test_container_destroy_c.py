"""Native state checks for the combined container destructor candidate."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class ContainerDestroyCandidateTests(unittest.TestCase):
    def test_fixed_traces_callback_images_and_final_state(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'container-destroy-test'
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'tests/c/container_destroy_harness.c'),
                str(ROOT / 'src/nonmatching/container_destroy.c'),
                str(ROOT / 'src/list_free.c'),
                '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
