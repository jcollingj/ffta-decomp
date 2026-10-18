"""Host checks for the independently compiled equipped-stat routines."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class UnitEquippedStatsCTests(unittest.TestCase):
    def test_fixed_images_traces_and_boundaries(self):
        with tempfile.TemporaryDirectory() as directory:
            program = Path(directory) / 'unit-equipped-stats-test'
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                '-c', str(ROOT / 'src/unit_equipped_stats.c'),
                '-o', str(Path(directory) / 'unit-equipped-stats.o'),
            ], check=True, capture_output=True, text=True)
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'tests/c/unit_equipped_stats_harness.c'),
                str(Path(directory) / 'unit-equipped-stats.o'), '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True, text=True)
