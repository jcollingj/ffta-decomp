"""Host checks for the two signed16-clamped unit equipment readers."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class UnitClampedValuesCTests(unittest.TestCase):
    def test_fixed_state_images_traces_and_narrowing(self):
        with tempfile.TemporaryDirectory() as directory:
            directory = Path(directory)
            production = directory / 'unit-clamped-values.o'
            program = directory / 'unit-clamped-values-test'
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'), '-c',
                str(ROOT / 'src/nonmatching/unit_clamped_values.c'),
                '-o', str(production),
            ], check=True, capture_output=True, text=True)
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'tests/c/unit_clamped_values_harness.c'),
                str(production), '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
