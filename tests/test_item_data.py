"""Use the real item authoring macros to check layout and typed edits."""
from pathlib import Path
import shutil
import unittest
import test_job_mapping_data as assembly

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which('arm-none-eabi-as') and shutil.which('arm-none-eabi-objcopy'),
                     'requires matching-build binutils')
class ItemDataTests(unittest.TestCase):
    assemble = assembly.JobMappingDataTests.assemble

    def source(self):
        return (ROOT / 'data/items.s').read_text()

    def replace_field(self, source, group, field, value):
        prefix = '    item_' + group + ' '
        old = next(line for line in source.splitlines() if line.startswith(prefix))
        values = old[len(prefix):].split(', ')
        values[field] = str(value)
        return source.replace(old, prefix + ', '.join(values), 1)

    def test_all_records_round_trip(self):
        result, actual = self.assemble(self.source())
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(len(actual), 376 * 32)
        self.assertEqual(actual, (ROOT / 'baserom.gba').read_bytes()[0x51d180:0x520080])

    def test_numeric_widths_and_ability_references_reject_overflow(self):
        cases = [('header', 1, -1, 'item_u16'), ('header', 1, 65536, 'item_u16'),
                 ('header', 4, -1, 'item_u8'), ('properties', 0, 256, 'item_u8'),
                 ('properties', 6, 65536, 'item_u16'),
                 ('modifiers', 0, -129, 'item_s8'), ('modifiers', 7, 128, 'item_s8'),
                 ('tail', 5, -1, 'must reference'), ('tail', 5, 225, 'must reference'),
                 ('tail', 7, 256, 'item_u8')]
        for group, field, value, diagnostic in cases:
            with self.subTest(group=group, field=field, value=value):
                result, _ = self.assemble(self.replace_field(self.source(), group, field, value))
                self.assertNotEqual(result.returncode, 0)
                self.assertIn(diagnostic, result.stderr)

    def test_record_order_and_extent_are_guarded(self):
        source = self.source()
        first_header = next(line for line in source.splitlines() if line.startswith('    item_header '))
        first_properties = next(line for line in source.splitlines() if line.startswith('    item_properties '))
        variants = [self.replace_field(source, 'header', 0, 1),
                    source.replace(first_header + '\n', '', 1),
                    source.replace(first_properties, first_properties + '\n' + first_properties, 1)]
        # Removing the complete final record must fail even though field groups align.
        final_start = source.index('    item_header 375,')
        final_end = source.index('.if (. - gItems) != 376 * 32', final_start)
        variants.append(source[:final_start] + source[final_end:])
        for variant in variants:
            result, _ = self.assemble(variant)
            self.assertNotEqual(result.returncode, 0)

    def test_legal_boundary_edits_preserve_other_bytes(self):
        source = self.source()
        for group, field, value in [('header', 1, 65535), ('header', 4, 255),
                                    ('modifiers', 0, -128), ('modifiers', 7, 127),
                                    ('tail', 5, 224)]:
            source = self.replace_field(source, group, field, value)
        result, actual = self.assemble(source)
        self.assertEqual(result.returncode, 0, result.stderr)
        expected = bytearray((ROOT / 'baserom.gba').read_bytes()[0x51d180:0x520080])
        expected[0:2] = b'\xff\xff'
        expected[6] = 255
        expected[16] = 128
        expected[23] = 127
        expected[29] = 224
        self.assertEqual(actual, expected)
