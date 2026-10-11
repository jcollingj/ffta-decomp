"""Actual mask assembly and original job-reference bounds."""
from pathlib import Path
import shutil
import struct
import unittest
import test_job_mapping_data as assembly

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('arm-none-eabi-as') and shutil.which('arm-none-eabi-objcopy'),
                     'requires matching-build binutils')
class EquipmentMaskDataTests(unittest.TestCase):
    assemble = assembly.JobMappingDataTests.assemble

    def source(self):
        return (ROOT / 'data/equipment_class_masks.s').read_text()

    def test_round_trip_and_original_job_references(self):
        result, actual = self.assemble(self.source())
        self.assertEqual(result.returncode, 0, result.stderr)
        base = (ROOT / 'baserom.gba').read_bytes()
        self.assertEqual(actual, base[0x51d0f4:0x51d180])
        indices = base[0x521a14 + 45:0x5231a4:52]
        self.assertEqual(len(indices), 116)
        self.assertEqual(set(indices), set(range(35)))
        self.assertTrue(all(index < len(actual) // 4 for index in indices))

    def test_rejects_invalid_width_order_and_extent(self):
        original = '    equipment_class_mask 0, 0x00000000\n'
        for replacement, message in [
            ('    equipment_class_mask 0, -1\n', 'unsigned word'),
            ('    equipment_class_mask 0, 0x100000000\n', 'unsigned word'),
            ('    equipment_class_mask 1, 0\n', 'consecutive'),
            ('', '35 records'),
        ]:
            with self.subTest(replacement=replacement):
                result, _ = self.assemble(self.source().replace(original, replacement, 1))
                self.assertNotEqual(result.returncode, 0)
                self.assertIn(message, result.stderr)

    def test_bounded_edit_preserves_other_masks(self):
        result, actual = self.assemble(self.source().replace(
            'equipment_class_mask 0, 0x00000000', 'equipment_class_mask 0, 0x80000001', 1))
        self.assertEqual(result.returncode, 0, result.stderr)
        expected = bytearray((ROOT / 'baserom.gba').read_bytes()[0x51d0f4:0x51d180])
        struct.pack_into('<I', expected, 0, 0x80000001)
        self.assertEqual(actual, expected)
