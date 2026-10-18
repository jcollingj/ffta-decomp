"""Assembled pointer spans and authoring guard rejection tests."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
TABLES = ((0, 0x5680DC, 725), (1, 0x5516D0, 107))


@unittest.skipUnless(shutil.which("arm-none-eabi-as") and shutil.which("arm-none-eabi-objcopy"),
                     "requires ARM binutils")
class UnitReferenceDataTests(unittest.TestCase):
    def assemble(self, source, output):
        return subprocess.run(["arm-none-eabi-as", "-mcpu=arm7tdmi", "-o", str(output), str(source)],
                              capture_output=True, text=True)

    def test_explicit_pointer_tables_match_original_spans(self):
        rom = (ROOT / "baserom.gba").read_bytes()
        with tempfile.TemporaryDirectory() as temp:
            for number, start, count in TABLES:
                with self.subTest(table=number):
                    obj = Path(temp) / f"table{number}.o"
                    raw = Path(temp) / f"table{number}.bin"
                    result = self.assemble(ROOT / f"data/unit_reference_table{number}.s", obj)
                    self.assertEqual(result.returncode, 0, result.stderr)
                    subprocess.run(["arm-none-eabi-objcopy", "-O", "binary", "-j", ".text",
                                    str(obj), str(raw)], check=True, capture_output=True)
                    self.assertEqual(len(raw.read_bytes()), count * 4)
                    self.assertEqual(raw.read_bytes(), rom[start:start + count * 4])

    def test_invalid_pointer_and_count_edits_fail_assembly(self):
        original = (ROOT / "data/unit_reference_table1.s").read_text()
        first = next(line for line in original.splitlines() if "reference_pointer 0x" in line)
        mutations = [original.replace(first, "    reference_pointer 0x07000000"),
                     original.replace(first, "    reference_pointer 0x09000000"),
                     original.replace(first + "\n", ""),
                     original.replace(first, first + "\n" + first)]
        with tempfile.TemporaryDirectory() as temp:
            source, obj = Path(temp) / "edited.s", Path(temp) / "edited.o"
            for text in mutations:
                source.write_text(text)
                result = self.assemble(source, obj)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("Error", result.stderr)
