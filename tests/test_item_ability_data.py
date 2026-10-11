"""Round-trip and authoring guards for recovered item ability-set records."""
from pathlib import Path
import shutil
import unittest
import test_job_mapping_data as assembly

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('arm-none-eabi-as') and shutil.which('arm-none-eabi-objcopy'),
                     'requires matching-build binutils')
class ItemAbilityDataTests(unittest.TestCase):
    assemble = assembly.JobMappingDataTests.assemble

    def source(self):
        return (ROOT / 'data/item_ability_sets.s').read_text()

    def test_round_trip_and_item_reference_bounds(self):
        result, actual = self.assemble(self.source())
        self.assertEqual(result.returncode, 0, result.stderr)
        base = (ROOT / 'baserom.gba').read_bytes()
        self.assertEqual(actual, base[0x520080:0x521214])
        self.assertEqual(len(actual), 225 * 20)
        indices = base[0x51d180 + 29:0x520080:32]
        self.assertEqual(len(indices), 376)
        self.assertEqual(max(indices), 224)
        self.assertTrue(all(index < len(actual) // 20 for index in indices))
        for offset in range(0, len(actual), 20):
            count = actual[offset]
            self.assertLessEqual(count, 9)
            self.assertEqual(actual[offset + 2 + count * 2:offset + 20],
                             bytes(18 - count * 2))

    def test_invalid_counts_and_byte_fields_are_rejected(self):
        for values, diagnostic in [
            ('-1', 'within nine pairs'), ('10', 'within nine pairs'),
            ('0, -1', 'fit unsigned bytes'), ('0, 256', 'fit unsigned bytes'),
            ('1, 0, -1, 0', 'fit unsigned bytes'),
            ('1, 0, 256, 0', 'fit unsigned bytes'),
            ('1, 0, 1, -1', 'fit unsigned bytes'),
            ('1, 0, 1, 256', 'fit unsigned bytes'),
        ]:
            with self.subTest(values=values):
                result, _ = self.assemble(self.source().replace(
                    '    item_ability_set 0, 0 @ 0', f'    item_ability_set {values} @ 0'))
                self.assertNotEqual(result.returncode, 0)
                self.assertIn(diagnostic, result.stderr)

    def test_fixed_table_extent_rejects_added_or_missing_record(self):
        for replacement in ['', '    item_ability_set 0\n    item_ability_set 0']:
            with self.subTest(replacement=replacement):
                result, _ = self.assemble(self.source().replace(
                    '    item_ability_set 0, 0 @ 0', replacement))
                self.assertNotEqual(result.returncode, 0)
                self.assertIn('must contain 225 records', result.stderr)

    def test_valid_edit_changes_only_selected_record_fields(self):
        result, actual = self.assemble(self.source().replace(
            '    item_ability_set 0, 0 @ 0', '    item_ability_set 1, 0, 255, 7 @ 0'))
        self.assertEqual(result.returncode, 0, result.stderr)
        expected = bytearray((ROOT / 'baserom.gba').read_bytes()[0x520080:0x521214])
        expected[0] = 1
        expected[2:4] = bytes([255, 7])
        self.assertEqual(actual, expected)
