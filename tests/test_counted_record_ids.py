"""Actual assembler round trips and authoring guards for the sentinel ID list."""
from pathlib import Path
import shutil
import struct
import unittest
import test_job_mapping_data as assembly

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('arm-none-eabi-as') and shutil.which('arm-none-eabi-objcopy'),
                     'requires matching-build binutils')
class CountedRecordIdsTests(unittest.TestCase):
    assemble = assembly.JobMappingDataTests.assemble

    def source(self):
        return (ROOT / 'data/counted_record_ids.s').read_text()

    def test_round_trip_and_single_terminal_sentinel(self):
        result, actual = self.assemble(self.source())
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(actual, (ROOT / 'baserom.gba').read_bytes()[0x529ee4:0x52a13e])
        ids = struct.unpack('<301H', actual)
        self.assertEqual(ids[-1], 65535)
        self.assertNotIn(65535, ids[:-1])

    def test_rejects_out_of_range_ids_and_early_sentinel(self):
        for value in [-1, 65535, 65536]:
            with self.subTest(value=value):
                result, _ = self.assemble(self.source().replace(
                    '    counted_record 3\n', f'    counted_record {value}\n', 1))
                self.assertNotEqual(result.returncode, 0)
                self.assertIn('must not be the terminator', result.stderr)

    def test_fixed_extent(self):
        for replacement in ['', '    counted_record 3\n    counted_record 4\n']:
            result, _ = self.assemble(self.source().replace('    counted_record 3\n', replacement, 1))
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('300 IDs and one terminator', result.stderr)

    def test_edit_changes_only_selected_halfword(self):
        for value in [0, 65534]:
            with self.subTest(value=value):
                result, actual = self.assemble(self.source().replace(
                    '    counted_record 3\n', f'    counted_record {value}\n', 1))
                self.assertEqual(result.returncode, 0, result.stderr)
                expected = bytearray((ROOT / 'baserom.gba').read_bytes()[0x529ee4:0x52a13e])
                struct.pack_into('<H', expected, 0, value)
                self.assertEqual(actual, expected)
