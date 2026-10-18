"""Focused lossless tests for the bounded content source entry codec."""

import copy
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools" / "decomp"))
import content_entries as codec  # noqa: E402


class ContentEntryCodecTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        codec.OUTPUT_ROOT.mkdir(parents=True, exist_ok=True)

    def test_unique_byte_entry_identity_and_each_known_byte_edit(self):
        raw = bytes(range(codec.ENTRY_SIZE))
        decoded = codec.decode_entry(raw)
        self.assertEqual(codec.encode_entry(decoded), raw)
        for field, offset in (("byte5", 5), ("byte6", 6), ("byte7", 7)):
            changed = dict(decoded)
            changed[field] ^= 0x80
            rebuilt = codec.encode_entry(changed)
            self.assertEqual([i for i, pair in enumerate(zip(raw, rebuilt))
                              if pair[0] != pair[1]], [offset])
            self.assertEqual(rebuilt[:5], raw[:5])
            self.assertEqual(rebuilt[8:], raw[8:])

    def test_all_byte_values_roundtrip_and_booleans_are_not_integers(self):
        for value in range(256):
            entry = {"opaque_prefix_hex": "0011223344", "byte5": value,
                     "byte6": value, "byte7": value,
                     "equipment_ids": [0, 1, 0x1234, 0xFFFF, 0],
                     "opaque_suffix_hex": "a5" * 30}
            self.assertEqual(codec.decode_entry(codec.encode_entry(entry)), entry)
        for field in ("byte5", "byte6", "byte7"):
            entry[field] = True
            with self.assertRaises(codec.ContentEntryError):
                codec.encode_entry(entry)

    def test_versioned_complete_roundtrips_and_legacy_payload_compatibility(self):
        raw = bytes(range(codec.ENTRY_SIZE))
        for version in (1, 2):
            decoded = codec.decode_entry(raw, version=version)
            self.assertEqual(codec.encode_entry(decoded, version=version), raw)
        old_document = codec.extract_entries((ROOT / "baserom.gba").read_bytes(),
                                             0x54CD7C)
        v2_payload = codec.rebuild_payload(old_document)
        v1_document = copy.deepcopy(old_document)
        v1_document["version"] = 1
        v1_document["entries"] = [codec.decode_entry(v2_payload[i:i + 48], version=1)
                                  for i in range(0, len(v2_payload), 48)]
        self.assertEqual(codec.rebuild_payload(v1_document), v2_payload)

    def test_equipment_little_endian_mapping_extremes_and_exact_edit_spans(self):
        raw = bytes(range(codec.ENTRY_SIZE))
        decoded = codec.decode_entry(raw)
        self.assertEqual(decoded["equipment_ids"], [0x0908, 0x0B0A, 0x0D0C,
                                                    0x0F0E, 0x1110])
        self.assertEqual(codec.encode_entry(decoded), raw)
        extremes = dict(decoded, equipment_ids=[0, 0xFFFF, 0, 0xFFFF, 0])
        encoded_extremes = codec.encode_entry(extremes)
        self.assertEqual(encoded_extremes[8:18], b"\0\0\xff\xff\0\0\xff\xff\0\0")
        self.assertEqual(codec.decode_entry(encoded_extremes)["equipment_ids"],
                         [0, 0xFFFF, 0, 0xFFFF, 0])
        for slot in range(5):
            edited = dict(decoded)
            ids = list(decoded["equipment_ids"])
            ids[slot] = 0xA5C3
            edited["equipment_ids"] = ids
            rebuilt = codec.encode_entry(edited)
            start = 8 + slot * 2
            self.assertEqual([i for i, pair in enumerate(zip(raw, rebuilt))
                              if pair[0] != pair[1]], [start, start + 1])
            self.assertEqual(rebuilt[:start], raw[:start])
            self.assertEqual(rebuilt[start + 2:], raw[start + 2:])

    def test_direct_entry_versions_and_equipment_values_are_strict(self):
        raw = bytes(range(codec.ENTRY_SIZE))
        v1 = codec.decode_entry(raw, version=1)
        v2 = codec.decode_entry(raw, version=2)
        for bad_version in (True, 1.0, 2.0, 0, 3):
            with self.subTest(version=bad_version):
                with self.assertRaises(codec.ContentEntryError):
                    codec.decode_entry(raw, version=bad_version)
                with self.assertRaises(codec.ContentEntryError):
                    codec.encode_entry(v2, version=bad_version)
        for bad_ids in ([True, 0, 0, 0, 0], [1.0, 0, 0, 0, 0],
                        [-1, 0, 0, 0, 0], [0x10000, 0, 0, 0, 0],
                        [0, 0, 0, 0], [0, 0, 0, 0, 0, 0]):
            malformed = dict(v2, equipment_ids=bad_ids)
            with self.subTest(ids=bad_ids):
                with self.assertRaises(codec.ContentEntryError):
                    codec.encode_entry(malformed)
        with self.assertRaises(codec.ContentEntryError):
            codec.encode_entry(dict(v1, equipment_ids=[0] * 5), version=1)
        with self.assertRaises(codec.ContentEntryError):
            codec.encode_entry({**v2, "extra": 1})

    def test_record_extent_pointer_and_payload_bounds(self):
        image = bytearray(64)
        image[0:2] = (1).to_bytes(2, "little")
        image[4:8] = codec.ROM_BASE.to_bytes(4, "little")
        self.assertEqual(codec.decode_rom_record(bytes(image), 0), (1, codec.ROM_BASE, 0))
        with self.assertRaisesRegex(codec.ContentEntryError, "40-byte record"):
            codec.decode_rom_record(bytes(image), 30)
        image[4:8] = (codec.ROM_BASE + codec.ROM_SIZE - 47).to_bytes(4, "little")
        with self.assertRaisesRegex(codec.ContentEntryError, "exceeds the 16 MiB"):
            codec.decode_rom_record(bytes(image), 0)
        image[4:8] = (codec.ROM_BASE + 32).to_bytes(4, "little")
        with self.assertRaisesRegex(codec.ContentEntryError, "payload is truncated"):
            codec.decode_rom_record(bytes(image), 0)
        image[4:8] = (0x06000000).to_bytes(4, "little")
        with self.assertRaisesRegex(codec.ContentEntryError, "does not translate"):
            codec.decode_rom_record(bytes(image), 0)
        image[0:2] = b"\0\0"
        image[4:8] = b"\0\0\0\0"
        self.assertEqual(codec.decode_rom_record(bytes(image), 0), (0, 0, None))

    def test_real_sample_spans_roundtrip_exactly(self):
        rom_path = ROOT / "baserom.gba"
        rom_before = hashlib.sha1(rom_path.read_bytes()).hexdigest()
        rom = rom_path.read_bytes()
        samples = (
            (0x54CD7C, 8, 0x0852A4D0, 0x52A4D0, 384,
             "fb31b5dc6170a16a0e2d956705543aa9e1ef8305",
             "6c73e7c84cfe0e2e2af6b98b019a4020ea8923fad45fede4e8a5b8aa3370bf04"),
            (0x54CDA4, 9, 0x0852A650, 0x52A650, 432,
             "6e5f57db1042576f56486b499042d0e50d2c6ab3",
             "60a9f3e7efd79227dbeb9e59d37515b2213ad518e9c8babece89adcc19afc645"),
        )
        for record, count, pointer, start, length, sha1, sha256 in samples:
            document = codec.extract_entries(rom, record)
            self.assertEqual(document["record_count"], count)
            self.assertEqual(document["source_pointer_hex"], f"0x{pointer:08X}")
            self.assertEqual(document["source_file_offset_hex"], f"0x{start:08X}")
            rebuilt = codec.rebuild_payload(document)
            original = rom[start:start + length]
            self.assertEqual(len(rebuilt), count * 48)
            self.assertEqual(rebuilt, original)
            self.assertEqual(hashlib.sha1(rebuilt).hexdigest(), sha1)
            self.assertEqual(hashlib.sha256(rebuilt).hexdigest(), sha256)
        self.assertEqual(hashlib.sha1(rom_path.read_bytes()).hexdigest(), rom_before)

    def test_extract_requires_fixed_full_rom_hash(self):
        bad = bytearray((ROOT / "baserom.gba").read_bytes())
        bad[-1] ^= 1
        with self.assertRaisesRegex(codec.ContentEntryError, "wrong ROM SHA-1"):
            codec.extract_entries(bytes(bad), 0x54CD7C)

    def test_rebuild_rejects_malformed_documents_and_entry_widths(self):
        document = codec.extract_entries((ROOT / "baserom.gba").read_bytes(), 0x54CD7C)
        mutations = []
        bad = copy.deepcopy(document); bad["version"] = True; mutations.append(bad)
        bad = copy.deepcopy(document); bad["version"] = 1.0; mutations.append(bad)
        bad = copy.deepcopy(document); bad["record_count"] = 7; mutations.append(bad)
        bad = copy.deepcopy(document); bad["source_file_offset_hex"] = "0x52A4D1"; mutations.append(bad)
        bad = copy.deepcopy(document); bad["source_pointer_hex"] = "0x06000000"; mutations.append(bad)
        bad = copy.deepcopy(document); bad["source_pointer_hex"] = "0x08FFFFD1"; bad["source_file_offset_hex"] = "0x00FFFFD1"; mutations.append(bad)
        bad = copy.deepcopy(document); bad["record_offset_hex"] = "0xFFFFF0"; mutations.append(bad)
        bad = copy.deepcopy(document); bad["entries"][0]["byte5"] = True; mutations.append(bad)
        bad = copy.deepcopy(document); bad["entries"][0]["byte6"] = -1; mutations.append(bad)
        bad = copy.deepcopy(document); bad["entries"][0]["byte7"] = 256; mutations.append(bad)
        bad = copy.deepcopy(document); bad["entries"][0]["opaque_prefix_hex"] = "0011"; mutations.append(bad)
        bad = copy.deepcopy(document); bad["entries"][0]["opaque_suffix_hex"] = "00" * 39; mutations.append(bad)
        bad = copy.deepcopy(document); bad["entries"][0]["equipment_ids"] = [True] * 5; mutations.append(bad)
        bad = copy.deepcopy(document); bad["entries"][0]["equipment_ids"] = [0] * 4; mutations.append(bad)
        bad = copy.deepcopy(document); bad["entries"][0]["equipment_ids"].append(0); mutations.append(bad)
        bad = copy.deepcopy(document); bad["entries"][0]["legacy_extra"] = 1; mutations.append(bad)
        bad = copy.deepcopy(document); bad["extra"] = 1; mutations.append(bad)
        bad = copy.deepcopy(document); bad["rom_sha1"] = "0" * 40; mutations.append(bad)
        for malformed in mutations:
            with self.subTest(document=malformed):
                with self.assertRaises(codec.ContentEntryError):
                    codec.rebuild_payload(malformed)

    def test_cli_extract_rebuild_and_atomic_no_clobber(self):
        rom = ROOT / "baserom.gba"
        with tempfile.TemporaryDirectory(dir=codec.OUTPUT_ROOT) as directory:
            json_path = Path(directory) / "sample.json"
            bin_path = Path(directory) / "sample.bin"
            prefix = [sys.executable, str(ROOT / "tools/decomp/content_entries.py")]
            subprocess.run(prefix + ["extract", str(rom), "--record-offset", "0x54CD7C",
                                     "--output", str(json_path)], check=True,
                           capture_output=True, text=True)
            document = json.loads(json_path.read_text(encoding="utf-8"))
            self.assertEqual(document["record_count"], 8)
            self.assertEqual(document["version"], 2)
            subprocess.run(prefix + ["rebuild", str(json_path), "--output", str(bin_path)],
                           check=True, capture_output=True, text=True)
            self.assertEqual(bin_path.read_bytes(), rom.read_bytes()[0x52A4D0:0x52A650])
            with self.assertRaises(subprocess.CalledProcessError):
                subprocess.run(prefix + ["rebuild", str(json_path), "--output", str(bin_path)],
                               check=True, capture_output=True, text=True)
            self.assertEqual(bin_path.stat().st_size, 384)
            failed_output = Path(directory) / "bad.bin"
            broken = copy.deepcopy(document)
            broken["entries"][0]["byte5"] = True
            broken_json = Path(directory) / "bad.json"
            broken_json.write_text(json.dumps(broken), encoding="utf-8")
            with self.assertRaises(subprocess.CalledProcessError):
                subprocess.run(prefix + ["rebuild", str(broken_json), "--output",
                                         str(failed_output)], check=True,
                               capture_output=True, text=True)
            self.assertFalse(failed_output.exists())

    def test_cli_output_rejects_escape_and_symlink(self):
        with tempfile.TemporaryDirectory(dir=codec.OUTPUT_ROOT) as directory:
            outside = codec.OUTPUT_ROOT.parent / "content-entry-escape-test"
            with self.assertRaises(codec.ContentEntryError):
                codec._write_new_file(str(outside), b"x")
            self.assertFalse(outside.exists())
            target = Path(directory) / "link.bin"
            target.symlink_to(Path(directory) / "missing.bin")
            with self.assertRaises(codec.ContentEntryError):
                codec._write_new_file(str(target), b"x")
            self.assertFalse((Path(directory) / "missing.bin").exists())

    def test_symlinked_output_ancestor_does_not_create_outside_directories(self):
        with tempfile.TemporaryDirectory(dir=codec.OUTPUT_ROOT) as directory:
            with tempfile.TemporaryDirectory() as outside:
                outside_path = Path(outside)
                link = Path(directory) / "escape"
                link.symlink_to(outside_path, target_is_directory=True)
                destination = link / "new" / "nested" / "payload.bin"
                with self.assertRaises(codec.ContentEntryError):
                    codec._write_new_file(str(destination), b"payload")
                self.assertEqual(list(outside_path.iterdir()), [])


if __name__ == "__main__":
    unittest.main()
