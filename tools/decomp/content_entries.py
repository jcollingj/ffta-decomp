#!/usr/bin/env python3
"""Losslessly inspect evidenced fields in 48-byte content entries."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import tempfile
from typing import Any


ROM_SHA1 = "4ac05441f4de70a4ec3dd932116346c61b8783d9"
ROM_SIZE = 0x1000000
ROM_BASE = 0x08000000
RECORD_SIZE = 40
ENTRY_SIZE = 48
FORMAT = "ffta-content-source-entries"
LEGACY_VERSION = 1
VERSION = 2
REPO_ROOT = Path(__file__).resolve().parents[2]
OUTPUT_ROOT = REPO_ROOT / "build" / "content-entries"


class ContentEntryError(ValueError):
    """Invalid ROM data, JSON, or output request."""


def _integer(value: Any, name: str, minimum: int, maximum: int) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise ContentEntryError(f"{name} must be an integer")
    if value < minimum or value > maximum:
        raise ContentEntryError(f"{name} must be in {minimum}..{maximum}")
    return value


def _hex_address(value: Any, name: str, *, nullable: bool = False) -> int | None:
    if nullable and value is None:
        return None
    if not isinstance(value, str) or re.fullmatch(r"0x[0-9a-fA-F]+", value) is None:
        raise ContentEntryError(f"{name} must be a hexadecimal address")
    return int(value, 16)


def _entry_version(version: Any) -> int:
    version = _integer(version, "version", LEGACY_VERSION, VERSION)
    if version not in (LEGACY_VERSION, VERSION):
        raise ContentEntryError("unsupported entry version")
    return version


def encode_entry(entry: Any, index: int = 0, version: int = VERSION) -> bytes:
    version = _entry_version(version)
    label = f"entries[{index}]"
    expected_keys = {"opaque_prefix_hex", "byte5", "byte6", "byte7",
                     "opaque_suffix_hex"}
    if version == VERSION:
        expected_keys.add("equipment_ids")
    if not isinstance(entry, dict) or set(entry) != expected_keys:
        raise ContentEntryError(f"{label} has an invalid shape")
    prefix = entry["opaque_prefix_hex"]
    suffix = entry["opaque_suffix_hex"]
    if not isinstance(prefix, str) or re.fullmatch(r"[0-9a-fA-F]{10}", prefix) is None:
        raise ContentEntryError(f"{label}.opaque_prefix_hex must encode exactly 5 bytes")
    suffix_bytes = 40 if version == LEGACY_VERSION else 30
    if (not isinstance(suffix, str)
            or re.fullmatch(rf"[0-9a-fA-F]{{{suffix_bytes * 2}}}", suffix) is None):
        raise ContentEntryError(
            f"{label}.opaque_suffix_hex must encode exactly {suffix_bytes} bytes")
    fields = bytes(_integer(entry[f"byte{n}"], f"{label}.byte{n}", 0, 255)
                   for n in (5, 6, 7))
    if version == LEGACY_VERSION:
        return bytes.fromhex(prefix) + fields + bytes.fromhex(suffix)
    equipment_ids = entry["equipment_ids"]
    if not isinstance(equipment_ids, list) or len(equipment_ids) != 5:
        raise ContentEntryError(f"{label}.equipment_ids must contain exactly 5 IDs")
    encoded_ids = b"".join(
        _integer(item_id, f"{label}.equipment_ids[{slot}]", 0, 0xFFFF).to_bytes(2, "little")
        for slot, item_id in enumerate(equipment_ids)
    )
    return (bytes.fromhex(prefix) + fields + encoded_ids + bytes.fromhex(suffix))


def decode_entry(raw: bytes, version: int = VERSION) -> dict[str, Any]:
    version = _entry_version(version)
    if len(raw) != ENTRY_SIZE:
        raise ContentEntryError("source entry must be exactly 48 bytes")
    entry = {
        "opaque_prefix_hex": raw[:5].hex(),
        "byte5": raw[5],
        "byte6": raw[6],
        "byte7": raw[7],
    }
    if version == LEGACY_VERSION:
        entry["opaque_suffix_hex"] = raw[8:].hex()
        return entry
    entry["equipment_ids"] = [int.from_bytes(raw[offset:offset + 2], "little")
                              for offset in range(8, 18, 2)]
    entry["opaque_suffix_hex"] = raw[18:].hex()
    return entry


def decode_rom_record(rom: bytes, record_offset: int) -> tuple[int, int, int | None]:
    """Read the observed 40-byte record and validate its bounded source span."""
    record_offset = _integer(record_offset, "record_offset", 0, ROM_SIZE - RECORD_SIZE)
    if record_offset + RECORD_SIZE > len(rom):
        raise ContentEntryError("the complete 40-byte record is truncated")
    count = int.from_bytes(rom[record_offset:record_offset + 2], "little")
    pointer = int.from_bytes(rom[record_offset + 4:record_offset + 8], "little")
    if pointer == 0 and count == 0:
        return count, pointer, None
    if pointer < ROM_BASE or pointer > ROM_BASE + ROM_SIZE:
        raise ContentEntryError("source pointer does not translate into the 16 MiB ROM")
    source_offset = pointer - ROM_BASE
    payload_length = count * ENTRY_SIZE
    if source_offset + payload_length > ROM_SIZE:
        raise ContentEntryError("counted source payload exceeds the 16 MiB ROM")
    if source_offset + payload_length > len(rom):
        raise ContentEntryError("counted source payload is truncated")
    return count, pointer, source_offset


def extract_entries(rom: bytes, record_offset: int) -> dict[str, Any]:
    """Decode one explicitly selected record from the expected USA ROM."""
    if len(rom) != ROM_SIZE:
        raise ContentEntryError(f"ROM must be exactly {ROM_SIZE} bytes")
    digest = hashlib.sha1(rom).hexdigest()
    if digest != ROM_SHA1:
        raise ContentEntryError(f"wrong ROM SHA-1: expected {ROM_SHA1}, got {digest}")
    count, pointer, source_offset = decode_rom_record(rom, record_offset)
    start = source_offset if source_offset is not None else 0
    entries = [
        decode_entry(rom[start + i * ENTRY_SIZE:start + (i + 1) * ENTRY_SIZE])
        for i in range(count)
    ]
    return {
        "format": FORMAT,
        "version": VERSION,
        "rom_sha1": ROM_SHA1,
        "record_offset_hex": f"0x{record_offset:08X}",
        "record_count": count,
        "source_pointer_hex": f"0x{pointer:08X}",
        "source_file_offset_hex": (None if source_offset is None
                                    else f"0x{source_offset:08X}"),
        "entries": entries,
    }


def rebuild_payload(document: Any) -> bytes:
    """Validate a versioned extraction document and rebuild its bounded bytes."""
    expected_keys = {
        "format", "version", "rom_sha1", "record_offset_hex", "record_count",
        "source_pointer_hex", "source_file_offset_hex", "entries",
    }
    if not isinstance(document, dict) or set(document) != expected_keys:
        raise ContentEntryError("document has an invalid top-level shape")
    if document["format"] != FORMAT:
        raise ContentEntryError("unsupported format or version")
    version = _entry_version(document["version"])
    if version not in (LEGACY_VERSION, VERSION):
        raise ContentEntryError("unsupported format or version")
    if document["rom_sha1"] != ROM_SHA1:
        raise ContentEntryError("document ROM SHA-1 does not match the supported USA ROM")
    record_offset = _hex_address(document["record_offset_hex"], "record_offset_hex")
    assert record_offset is not None
    if record_offset + RECORD_SIZE > ROM_SIZE:
        raise ContentEntryError("record range exceeds the 16 MiB ROM")
    count = _integer(document["record_count"], "record_count", 0, 0xFFFF)
    entries = document["entries"]
    if not isinstance(entries, list) or len(entries) != count:
        raise ContentEntryError("record_count must equal the number of entries")
    pointer = _hex_address(document["source_pointer_hex"], "source_pointer_hex")
    source_offset = _hex_address(document["source_file_offset_hex"],
                                 "source_file_offset_hex", nullable=True)
    assert pointer is not None
    if pointer == 0 and count == 0:
        if source_offset is not None:
            raise ContentEntryError("null source pointer requires a null source offset")
    else:
        if pointer < ROM_BASE or pointer > ROM_BASE + ROM_SIZE:
            raise ContentEntryError("source pointer does not translate into the 16 MiB ROM")
        translated = pointer - ROM_BASE
        if source_offset != translated:
            raise ContentEntryError("source pointer and source file offset disagree")
        if translated + count * ENTRY_SIZE > ROM_SIZE:
            raise ContentEntryError("counted source payload exceeds the 16 MiB ROM")
    return b"".join(encode_entry(entry, i, version) for i, entry in enumerate(entries))


def _reject_duplicate_pairs(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise ContentEntryError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def _safe_output_path(raw_path: str) -> Path:
    path = Path(raw_path)
    if not path.is_absolute():
        path = REPO_ROOT / path
    lexical_base = Path(os.path.abspath(OUTPUT_ROOT))
    for root_part in (REPO_ROOT / "build", OUTPUT_ROOT):
        if root_part.is_symlink():
            raise ContentEntryError("build/content-entries may not be a symlink")
    base = OUTPUT_ROOT.resolve()
    if base != lexical_base:
        raise ContentEntryError("build/content-entries resolves outside its fixed path")
    parent = path.parent
    lexical_parent = Path(os.path.abspath(parent))
    try:
        lexical_parent.relative_to(lexical_base)
    except ValueError as error:
        raise ContentEntryError(f"output must be under {OUTPUT_ROOT}") from error
    resolved_parent = parent.resolve(strict=False)
    try:
        resolved_parent.relative_to(base)
    except ValueError as error:
        raise ContentEntryError(f"output must be under {OUTPUT_ROOT}") from error
    parent.mkdir(parents=True, exist_ok=True)
    resolved_parent = parent.resolve()
    try:
        resolved_parent.relative_to(base)
    except ValueError as error:
        raise ContentEntryError(f"output must be under {OUTPUT_ROOT}") from error
    if path.name in ("", ".", ".."):
        raise ContentEntryError("output must name a file")
    return resolved_parent / path.name


def _write_new_file(raw_path: str, data: bytes) -> Path:
    target = _safe_output_path(raw_path)
    if os.path.lexists(target):
        raise ContentEntryError(f"refusing to overwrite existing output: {target}")
    fd, temporary_name = tempfile.mkstemp(prefix=".content-entry-", dir=target.parent)
    temporary = Path(temporary_name)
    try:
        with os.fdopen(fd, "wb") as stream:
            stream.write(data)
            stream.flush()
            os.fsync(stream.fileno())
        # Hard-link publication is atomic and fails if another writer won the name.
        os.link(temporary, target)
        temporary.unlink()
        return target
    except FileExistsError as error:
        raise ContentEntryError(f"refusing to overwrite existing output: {target}") from error
    finally:
        try:
            temporary.unlink()
        except FileNotFoundError:
            pass


def _extract_command(args: argparse.Namespace) -> None:
    rom = Path(args.rom).read_bytes()
    document = extract_entries(rom, args.record_offset)
    rendered = (json.dumps(document, indent=2, sort_keys=True) + "\n").encode("utf-8")
    target = _write_new_file(args.output, rendered)
    print(target)


def _rebuild_command(args: argparse.Namespace) -> None:
    try:
        document = json.loads(Path(args.document).read_text(encoding="utf-8"),
                              object_pairs_hook=_reject_duplicate_pairs)
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise ContentEntryError(f"cannot read JSON document: {error}") from error
    target = _write_new_file(args.output, rebuild_payload(document))
    print(target)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)
    extract = subparsers.add_parser("extract", help="extract one caller-selected record")
    extract.add_argument("rom")
    extract.add_argument("--record-offset", required=True, type=lambda value: int(value, 0))
    extract.add_argument("--output", required=True)
    extract.set_defaults(run=_extract_command)
    rebuild = subparsers.add_parser("rebuild", help="rebuild a bounded payload from JSON")
    rebuild.add_argument("document")
    rebuild.add_argument("--output", required=True)
    rebuild.set_defaults(run=_rebuild_command)
    args = parser.parse_args(argv)
    try:
        args.run(args)
    except (ContentEntryError, OSError, ValueError) as error:
        parser.exit(2, f"error: {error}\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
