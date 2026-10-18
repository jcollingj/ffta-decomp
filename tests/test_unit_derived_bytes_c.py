"""Host checks for the recovered derived-byte updater and reader."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('cc'), 'requires a host C compiler')
class UnitDerivedBytesCTests(unittest.TestCase):
    def compile_and_run(self, directory, name, sources, defines=()):
        program = Path(directory) / name
        command = [
            'cc', '-std=gnu89', '-Wall', '-Wextra', '-Werror',
            '-I', str(ROOT / 'include'),
        ]
        command.extend(f'-D{define}' for define in defines)
        command.extend(str(ROOT / source) for source in sources)
        command.extend(['-o', str(program)])
        subprocess.run(command, check=True, capture_output=True, text=True)
        subprocess.run([str(program)], check=True, capture_output=True, text=True)

    def test_controlled_boundaries_and_whole_record_images(self):
        with tempfile.TemporaryDirectory() as directory:
            self.compile_and_run(directory, 'controlled', [
                'src/unit_derived_bytes.c',
                'src/unit_derived_byte_read.c',
                'tests/c/unit_derived_bytes_harness.c',
            ])

    def test_real_status_and_unit_property_composition(self):
        with tempfile.TemporaryDirectory() as directory:
            self.compile_and_run(directory, 'composition', [
                'src/unit_derived_bytes.c',
                'src/unit_derived_byte_read.c',
                'src/unit_property.c',
                'src/unit_status.c',
                'tests/c/unit_derived_bytes_harness.c',
            ], defines=('REAL_COMPOSITION',))
