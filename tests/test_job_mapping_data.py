"""Assemble actual authoring input: lossless records and rejected invalid edits."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('arm-none-eabi-as') and shutil.which('arm-none-eabi-objcopy'),
                     'requires matching-build binutils')
class JobMappingDataTests(unittest.TestCase):
    def assemble(self, source):
        with tempfile.TemporaryDirectory() as directory:
            folder = Path(directory)
            (folder / 'data.s').write_text(source)
            result = subprocess.run(['arm-none-eabi-as', '-o', str(folder / 'data.o'),
                                     str(folder / 'data.s')], capture_output=True, text=True)
            if result.returncode:
                return result, None
            subprocess.run(['arm-none-eabi-objcopy', '-O', 'binary',
                            str(folder / 'data.o'), str(folder / 'data.bin')], check=True)
            return result, (folder / 'data.bin').read_bytes()

    def source(self):
        return (ROOT / 'data/job_mapping_data.s').read_text()

    def test_authored_records_reproduce_original_region(self):
        result, actual = self.assemble(self.source())
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(len(actual), 86)
        self.assertEqual(actual, (ROOT / 'baserom.gba').read_bytes()[0x5231f4:0x52324a])

    def test_out_of_range_fields_fail_instead_of_truncating(self):
        for key, value in [(-1, 0), (256, 0), (0, -1), (0, 256)]:
            with self.subTest(key=key, value=value):
                result, _ = self.assemble(self.source().replace(
                    '    job_mapping 0, 0', f'    job_mapping {key}, {value}'))
                self.assertNotEqual(result.returncode, 0)
                self.assertIn('must fit unsigned bytes', result.stderr)

    def test_record_removal_or_addition_fails_matching_layout(self):
        for replacement in ['', '    job_mapping 0, 0\n    job_mapping 0, 0']:
            with self.subTest(replacement=replacement):
                result, _ = self.assemble(self.source().replace('    job_mapping 0, 0', replacement))
                self.assertNotEqual(result.returncode, 0)
                self.assertIn('must contain 43 records', result.stderr)


@unittest.skipUnless(shutil.which('arm-none-eabi-as') and shutil.which('arm-none-eabi-objcopy'),
                     'requires matching-build binutils')
class JobRequirementDataTests(unittest.TestCase):
    assemble = JobMappingDataTests.assemble

    def source(self):
        return (ROOT / 'data/job_requirements.s').read_text()

    def test_prerequisites_round_trip_and_job_references_fit(self):
        result, actual = self.assemble(self.source())
        self.assertEqual(result.returncode, 0, result.stderr)
        base = (ROOT / 'baserom.gba').read_bytes()
        self.assertEqual(actual, base[0x5231a4:0x5231f4])
        self.assertEqual(len(actual), 80)
        indices = {base[0x521a14 + i * 52 + 48] for i in range(116)}
        self.assertEqual(indices, set(range(20)))

    def test_invalid_prerequisite_fields_and_record_counts_fail(self):
        for replacement, diagnostic in [
            ('job_requirements 116, 0, 0, 0', 'within the original job table'),
            ('job_requirements 0, -1, 0, 0', 'fit unsigned bytes'),
            ('job_requirements 0, 0, -1, 0', 'within the original job table'),
            ('job_requirements 0, 0, 0, 256', 'fit unsigned bytes'),
            ('', 'must contain 20 records'),
        ]:
            with self.subTest(replacement=replacement):
                result, _ = self.assemble(self.source().replace(
                    'job_requirements 0, 0, 0, 0 @ 0', replacement))
                self.assertNotEqual(result.returncode, 0)
                self.assertIn(diagnostic, result.stderr)
