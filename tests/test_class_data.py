"""Bounded class-record decoding and exact GNU assembler round trips."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

from tools.decomp import class_data


ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(shutil.which("arm-none-eabi-as") and shutil.which("arm-none-eabi-objcopy"),
                     "requires matching-build binutils")
class ClassDataTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.rom = (ROOT / "baserom.gba").read_bytes()
        cls.raw4 = cls.rom[class_data.CLASS4_START:
                           class_data.CLASS4_START + class_data.CLASS4_COUNT * class_data.CLASS4_RECORD_SIZE]
        cls.raw16 = cls.rom[class_data.CLASS16_START:
                            class_data.CLASS16_START + class_data.CLASS16_COUNT * class_data.CLASS16_RECORD_SIZE]
        cls.records4, cls.records16 = class_data.decode_rom(cls.rom)
        cls.source4 = (ROOT / "data/class_records4.s").read_text()
        cls.source16 = (ROOT / "data/class_records16.s").read_text()

    def assemble(self, source, expected):
        with tempfile.TemporaryDirectory() as directory:
            folder = Path(directory)
            (folder / "class_records.s").write_text(source)
            assembled = subprocess.run(
                ["arm-none-eabi-as", "-o", str(folder / "class_records.o"), str(folder / "class_records.s")],
                capture_output=True, text=True,
            )
            if assembled.returncode:
                return assembled, None
            copied = subprocess.run(
                ["arm-none-eabi-objcopy", "-O", "binary", str(folder / "class_records.o"),
                 str(folder / "class_records.bin")],
                capture_output=True, text=True,
            )
            self.assertEqual(copied.returncode, 0, copied.stderr)
            result = (folder / "class_records.bin").read_bytes()
            self.assertEqual(result, expected)
            return assembled, result

    def test_bounded_records_round_trip_at_verified_offsets(self):
        self.assertEqual(class_data.CLASS4_START, 0x39475C)
        self.assertEqual(class_data.CLASS16_START, 0x528F34)
        self.assertEqual(class_data.CLASS4_COUNT, 7)
        self.assertEqual(class_data.CLASS16_COUNT, 86)
        self.assertEqual(class_data.encode_class4_table(self.records4), self.raw4)
        self.assertEqual(class_data.encode_class16_table(self.records16), self.raw16)
        self.assertEqual(class_data.render_class4_asm(self.records4), self.source4)
        self.assertEqual(class_data.render_class16_asm(self.records16), self.source16)
        assembled4, actual4 = self.assemble(self.source4, self.raw4)
        assembled16, actual16 = self.assemble(self.source16, self.raw16)
        self.assertEqual(assembled4.returncode, 0, assembled4.stderr)
        self.assertEqual(assembled16.returncode, 0, assembled16.stderr)
        self.assertEqual(len(actual4), 28)
        self.assertEqual(len(actual16), 86 * 16)

    def test_reader_offsets_and_little_endian_fields_are_independent(self):
        # Independently fixed bytes distinguish +0, +1, +2..3 and unknown tail.
        raw = bytes.fromhex("a1 b2 34 12 d5 e6 f7 08 19 2a 3b 4c 5d 6e 7f 80")
        record = class_data.decode_class16_record(raw)
        self.assertEqual(record["field00"], 0xA1)
        self.assertEqual(record["unknown01"], 0xB2)
        self.assertEqual(record["field02"], 0x1234)
        self.assertEqual(record["unknown04"], "d5e6f708192a3b4c5d6e7f80")
        self.assertEqual(class_data.encode_class16_record(record), raw)

        # The lookup reader's observed byte selector is offset zero, not an
        # index inferred from a host-side table or the adjacent halfword.
        self.assertEqual(class_data.decode_class16_record(self.raw16[3 * 16:4 * 16])["field00"],
                         self.raw16[3 * 16])
        self.assertEqual(class_data.decode_class16_record(self.raw16[3 * 16:4 * 16])["field02"],
                         int.from_bytes(self.raw16[3 * 16 + 2:3 * 16 + 4], "little"))

    def test_field_mutation_does_not_bleed_into_adjacent_bytes(self):
        raw = bytes.fromhex("a1 b2 34 12 d5 e6 f7 08 19 2a 3b 4c 5d 6e 7f 80")
        record = class_data.decode_class16_record(raw)
        record["field00"] = 0x5A
        changed = class_data.encode_class16_record(record)
        self.assertEqual(changed, bytes.fromhex("5a b2 34 12 d5 e6 f7 08 19 2a 3b 4c 5d 6e 7f 80"))
        record = class_data.decode_class16_record(raw)
        record["field02"] = 0xABCD
        changed = class_data.encode_class16_record(record)
        self.assertEqual(changed, bytes.fromhex("a1 b2 cd ab d5 e6 f7 08 19 2a 3b 4c 5d 6e 7f 80"))
        record = class_data.decode_class4_record(bytes.fromhex("11 22 33 44"))
        record["field00"] = 0xAA
        self.assertEqual(class_data.encode_class4_record(record), bytes.fromhex("aa 22 33 44"))

    def test_invalid_ranges_shapes_and_truncated_hex_are_rejected(self):
        good16 = class_data.decode_class16_record(bytes(range(16)))
        whitespace_short_hex = ("00" * 5) + "  " + ("00" * 6)
        self.assertEqual(len(whitespace_short_hex), 24)
        self.assertEqual(len(bytes.fromhex(whitespace_short_hex)), 11)
        for name, value in (("field00", -1), ("field00", 256), ("field02", 65536),
                            ("field02", -1), ("unknown04", "00" * 11),
                            ("unknown04", "0g" * 12),
                            ("unknown04", whitespace_short_hex)):
            with self.subTest(name=name, value=value):
                changed = dict(good16)
                changed[name] = value
                with self.assertRaises(ValueError):
                    class_data.encode_class16_record(changed)
        with self.assertRaises(ValueError):
            class_data.encode_class4_record({"field00": 256, "unknown01": 0,
                                             "unknown02": 0, "unknown03": 0})
        with self.assertRaises(ValueError):
            class_data.decode_class4_record(bytes(3))
        with self.assertRaises(ValueError):
            class_data.decode_class16_record(bytes(17))
        with self.assertRaises(ValueError):
            class_data.decode_class4_table(self.raw4[:-1])
        with self.assertRaises(ValueError):
            class_data.decode_class16_table(self.raw16[:-1])
        with self.assertRaises(ValueError):
            class_data.encode_class16_table(self.records16[:-1])
        with self.assertRaises(ValueError):
            class_data.decode_rom(self.rom[:class_data.CLASS16_START +
                                          class_data.CLASS16_COUNT * class_data.CLASS16_RECORD_SIZE - 1])

    def test_assembly_rejects_bad_record_values_and_missing_rows(self):
        first = next(line for line in self.source16.splitlines() if line.startswith("    class16_record 0,"))
        fields = first.split(", ")
        fields[1] = "256"
        bad_byte = self.source16.replace(first, ", ".join(fields), 1)
        assembled, _ = self.assemble(bad_byte, self.raw16)
        self.assertNotEqual(assembled.returncode, 0)

        fields = first.split(", ")
        fields[3] = "65536"
        bad_halfword = self.source16.replace(first, ", ".join(fields), 1)
        assembled, _ = self.assemble(bad_halfword, self.raw16)
        self.assertNotEqual(assembled.returncode, 0)

        last = next(line for line in self.source16.splitlines() if line.startswith("    class16_record 85,"))
        assembled, _ = self.assemble(self.source16.replace(last + "\n", "", 1), self.raw16)
        self.assertNotEqual(assembled.returncode, 0)


if __name__ == "__main__":
    unittest.main()
