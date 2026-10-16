"""Lossless authoring and bounded assembly guards for ability records 0..76."""
import hashlib
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

from tools.decomp import ability_data

ROOT = Path(__file__).resolve().parents[1]


def fixed_record():
    """Independent field vector; expected bytes below are manually placed."""
    return {
        "name_id": 0x1234,
        "category": 0x56,
        "unknown_03": 0x78,
        "mp_cost": 0x9A,
        "unknown_05": 0xBC,
        "unknown_06": 0xDE,
        "unknown_07": 0xF0,
        "range": 0x11,
        "area": 0x22,
        "unknown_0a": 0x33,
        "power": 0x44,
        "effects": "55667788",
        "flags": 0xA1B2C3D4,
        "unknown_14": 0xE5F6,
        "unknown_16": "102030405060",
    }


EXPECTED_FIXED = bytes.fromhex(
    "34 12 56 78 9a bc de f0 11 22 33 44 55 66 77 88 "
    "d4 c3 b2 a1 f6 e5 10 20 30 40 50 60"
)


class AbilityDataSchemaTests(unittest.TestCase):
    def test_independent_fixed_record_field_placement(self):
        self.assertEqual(ability_data.encode_record(fixed_record()), EXPECTED_FIXED)
        decoded = ability_data.decode_record(EXPECTED_FIXED)
        self.assertEqual(decoded, fixed_record())

    def test_decode_encode_round_trip_on_synthetic_fixed_table(self):
        raw = b"".join(bytes((i + j) & 0xFF for j in range(28)) for i in range(77))
        self.assertEqual(ability_data.encode_table(ability_data.decode_table(raw)), raw)

    def test_single_field_mutation_preserves_every_neighbor(self):
        records = [dict(fixed_record()) for _ in range(77)]
        before = ability_data.encode_table(records)
        records[19]["mp_cost"] ^= 0x40
        after = ability_data.encode_table(records)
        offset = 19 * 28 + 4
        expected = bytearray(before)
        expected[offset] ^= 0x40
        self.assertEqual(after, bytes(expected))

    def test_schema_raw_lengths_and_numeric_types_are_strict(self):
        good = fixed_record()
        for field in ("effects", "unknown_16"):
            for value in ("", "0", "00 00", "0g" * (4 if field == "effects" else 6)):
                bad = dict(good, **{field: value})
                with self.subTest(field=field, value=value), self.assertRaises(ValueError):
                    ability_data.encode_record(bad)
        for value in (True, False, 1.0, -1, 256):
            for field in ("category", "mp_cost", "power"):
                with self.subTest(field=field, value=value), self.assertRaises(ValueError):
                    ability_data.encode_record(dict(good, **{field: value}))
        for field, value in (("name_id", -1), ("name_id", 65536),
                             ("flags", -1), ("flags", 1 << 32),
                             ("unknown_14", 65536)):
            with self.subTest(field=field, value=value), self.assertRaises(ValueError):
                ability_data.encode_record(dict(good, **{field: value}))
        for field in ("effects", "flags", "unknown_16"):
            with self.subTest(missing=field), self.assertRaises(ValueError):
                ability_data.encode_record({k: v for k, v in good.items() if k != field})
        with self.assertRaises(ValueError):
            ability_data.encode_record(dict(good, speculative_semantic=1))

    def test_record_and_prefix_shapes_are_exact(self):
        with self.assertRaises(ValueError):
            ability_data.decode_record(EXPECTED_FIXED[:-1])
        with self.assertRaises(ValueError):
            ability_data.decode_table(bytes(ability_data.SIZE - 1))
        records = [fixed_record() for _ in range(77)]
        with self.assertRaises(ValueError):
            ability_data.encode_table(records[:-1])
        with self.assertRaises(ValueError):
            ability_data.encode_table(records + [fixed_record()])
        with self.assertRaises(ValueError):
            ability_data.render_asm(records[:-1])

    def test_extract_requires_exact_usa_provenance(self):
        with self.assertRaises(ValueError):
            ability_data.extract_usa_prefix(bytes(0x1000000))


@unittest.skipUnless(shutil.which("arm-none-eabi-as") and shutil.which("arm-none-eabi-objcopy"),
                     "requires matching-build binutils")
class AbilityDataAssemblyTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = (ROOT / "data/ability_prefix.s").read_text()

    def assemble(self, source):
        with tempfile.TemporaryDirectory() as directory:
            folder = Path(directory)
            (folder / "ability.s").write_text(source)
            result = subprocess.run(["arm-none-eabi-as", "-o", str(folder / "ability.o"),
                                     str(folder / "ability.s")], capture_output=True, text=True)
            if result.returncode:
                return result, None
            subprocess.run(["arm-none-eabi-objcopy", "-O", "binary", str(folder / "ability.o"),
                            str(folder / "ability.bin")], check=True)
            return result, (folder / "ability.bin").read_bytes()

    def test_authored_assembly_matches_verified_local_prefix(self):
        rom_path = ROOT / "baserom.gba"
        if not rom_path.exists():
            self.skipTest("ROM-dependent check: baserom.gba is absent")
        rom = rom_path.read_bytes()
        if len(rom) != 0x1000000 or hashlib.sha1(rom).hexdigest() != ability_data.USA_SHA1:
            self.skipTest("ROM-dependent check: baserom.gba is not the verified USA image")
        expected = ability_data.extract_usa_prefix(rom)
        result, actual = self.assemble(self.source)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(len(actual), 77 * 28)
        self.assertEqual(actual, expected)

    def test_assembly_rejects_invalid_values_and_wrong_count(self):
        first = next(line for line in self.source.splitlines() if line.startswith("    ability_record 0,"))
        signature = next(line for line in self.source.splitlines() if line.startswith(".macro ability_record "))
        params = signature.removeprefix(".macro ability_record ").split(", ")
        for field, value in (("category", "256"), ("name_id", "65536"),
                             ("flags", "4294967296"), ("effects_2", "-1")):
            with self.subTest(field=field):
                values = first.split(", ")
                values[params.index(field)] = value
                altered = self.source.replace(first, ", ".join(values), 1)
                result, _ = self.assemble(altered)
                self.assertNotEqual(result.returncode, 0)
        removed = self.source.replace(
            next(line + "\n" for line in self.source.splitlines()
                 if line.startswith("    ability_record 76,")), "", 1)
        result, _ = self.assemble(removed)
        self.assertNotEqual(result.returncode, 0)


if __name__ == "__main__":
    unittest.main()
