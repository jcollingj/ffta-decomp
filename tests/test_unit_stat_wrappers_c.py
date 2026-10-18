"""Host behavior checks for the eight unit stat wrappers."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class UnitStatWrappersCTests(unittest.TestCase):
    def test_dispatch_priority_caps_traces_and_full_fixture(self):
        with tempfile.TemporaryDirectory() as directory:
            temp = Path(directory)
            objects = []
            for source in (
                    'src/unit_stat_dispatch.c',
                    'src/unit_stat_caps.c'):
                obj = temp / (Path(source).stem + '.o')
                subprocess.run([
                    'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                    '-I', str(ROOT / 'include'), '-c', str(ROOT / source),
                    '-o', str(obj),
                ], check=True, capture_output=True, text=True)
                objects.append(str(obj))
            program = temp / 'unit-stat-wrappers-test'
            subprocess.run([
                'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
                '-I', str(ROOT / 'include'),
                str(ROOT / 'tests/c/unit_stat_wrappers_harness.c'),
                *objects, '-o', str(program),
            ], check=True, capture_output=True, text=True)
            subprocess.run([str(program)], check=True, capture_output=True,
                           text=True)
