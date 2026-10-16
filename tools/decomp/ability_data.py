"""Lossless source tools for the verified ability-definition prefix.

Only records 0..76 are authored here. The 77-record boundary is supported by
the recovered First Aid record used by the bounded gameplay experiment; it does
not assert an extent for the neighboring definitions.
"""
from __future__ import annotations

import argparse
import hashlib
from pathlib import Path

COUNT = 77
RECORD_SIZE = 28
START = 0x55187C
SIZE = COUNT * RECORD_SIZE
USA_SHA1 = "4ac05441f4de70a4ec3dd932116346c61b8783d9"

# Names and widths follow include/ability.h and the selector reads in
# GetAbilityProperty. Unknown bytes stay offset-named and raw arrays stay raw.
FIELDS = (
    ("name_id", 0, 2, "u16"),
    ("category", 2, 1, "u8"),
    ("unknown_03", 3, 1, "u8"),
    ("mp_cost", 4, 1, "u8"),
    ("unknown_05", 5, 1, "u8"),
    ("unknown_06", 6, 1, "u8"),
    ("unknown_07", 7, 1, "u8"),
    ("range", 8, 1, "u8"),
    ("area", 9, 1, "u8"),
    ("unknown_0a", 10, 1, "u8"),
    ("power", 11, 1, "u8"),
    ("effects", 12, 4, "raw"),
    ("flags", 16, 4, "u32"),
    ("unknown_14", 20, 2, "u16"),
    ("unknown_16", 22, 6, "raw"),
)
FIELD_MAP = {name: (offset, width, kind) for name, offset, width, kind in FIELDS}


def _check_fields(record: dict) -> None:
    if not isinstance(record, dict) or set(record) != set(FIELD_MAP):
        raise ValueError("record fields do not match the documented 28-byte schema")


def _range(name: str, value: object, bits: int) -> None:
    if type(value) is not int or not 0 <= value < (1 << bits):
        raise ValueError(f"{name} must fit unsigned {bits}-bit field")


def _raw(name: str, value: object, width: int) -> bytes:
    if not isinstance(value, str) or len(value) != width * 2:
        raise ValueError(f"{name} must be exactly {width} raw bytes as hexadecimal")
    try:
        raw = bytes.fromhex(value)
    except ValueError as exc:
        raise ValueError(f"{name} must be hexadecimal") from exc
    if len(raw) != width:
        raise ValueError(f"{name} must decode to exactly {width} bytes")
    return raw


def decode_record(data: bytes) -> dict:
    if not isinstance(data, (bytes, bytearray, memoryview)) or len(data) != RECORD_SIZE:
        raise ValueError(f"ability record must be exactly {RECORD_SIZE} bytes")
    record = {}
    for name, offset, width, kind in FIELDS:
        chunk = bytes(data[offset:offset + width])
        record[name] = chunk.hex() if kind == "raw" else int.from_bytes(chunk, "little")
    return record


def encode_record(record: dict) -> bytes:
    _check_fields(record)
    out = bytearray(RECORD_SIZE)
    for name, offset, width, kind in FIELDS:
        value = record[name]
        if kind == "raw":
            out[offset:offset + width] = _raw(name, value, width)
        else:
            bits = width * 8
            _range(name, value, bits)
            out[offset:offset + width] = value.to_bytes(width, "little")
    return bytes(out)


def decode_table(data: bytes) -> list[dict]:
    if not isinstance(data, (bytes, bytearray, memoryview)) or len(data) != SIZE:
        raise ValueError(f"ability prefix must be exactly {SIZE} bytes ({COUNT} records)")
    return [decode_record(data[i:i + RECORD_SIZE]) for i in range(0, SIZE, RECORD_SIZE)]


def encode_table(records) -> bytes:
    if not isinstance(records, (list, tuple)) or len(records) != COUNT:
        raise ValueError(f"ability prefix must contain exactly {COUNT} records")
    return b"".join(encode_record(record) for record in records)


def _asm_fields():
    # Keep aggregate raw fields byte-addressable in assembly so each byte is
    # visible and independently range checked, while Python exposes exact blobs.
    for name, _offset, width, kind in FIELDS:
        if kind == "raw":
            for index in range(width):
                yield f"{name}_{index}"
        else:
            yield name


def render_asm(records) -> str:
    if not isinstance(records, (list, tuple)) or len(records) != COUNT:
        raise ValueError(f"ability prefix must contain exactly {COUNT} records")
    for record in records:
        encode_record(record)

    lines = [
        "@ Authored ability definitions 0..76 only (77 records, 28 bytes each).",
        "@ Field names follow GetAbilityProperty; unknown bytes retain offset names.",
        ".section .text",
        ".global gAbilityPrefix",
        ".type gAbilityPrefix, %object",
        ".macro ability_u8 value",
        "    .if (\\value < 0) || (\\value > 255)",
        '        .error "ability byte out of range"',
        "    .endif",
        "    .byte \\value",
        ".endm",
        ".macro ability_u16 value",
        "    .if (\\value < 0) || (\\value > 65535)",
        '        .error "ability halfword out of range"',
        "    .endif",
        "    .hword \\value",
        ".endm",
        ".macro ability_u32 value",
        "    .if (\\value < 0) || (\\value > 4294967295)",
        '        .error "ability word out of range"',
        "    .endif",
        "    .word \\value",
        ".endm",
        ".macro ability_record id, " + ", ".join(_asm_fields()),
        "    .if (. - gAbilityPrefix) != (\\id * 28)",
        '        .error "ability IDs must be consecutive from zero"',
        "    .endif",
    ]
    for name, _offset, _width, kind in FIELDS:
        if kind == "raw":
            for index in range(_width):
                lines.append(f"    ability_u8 \\{name}_{index}")
        else:
            macro = {"u8": "ability_u8", "u16": "ability_u16", "u32": "ability_u32"}[kind]
            lines.append(f"    {macro} \\{name}")
    lines += [".endm", "gAbilityPrefix:"]
    for index, record in enumerate(records):
        values = []
        for name, _offset, _width, kind in FIELDS:
            if kind == "raw":
                values.extend(_raw(name, record[name], _width))
            else:
                values.append(record[name])
        lines.append(f"    ability_record {index}, " + ", ".join(map(str, values)))
        lines.append("")
    lines += [
        f".if (. - gAbilityPrefix) != {SIZE}",
        '    .error "ability prefix must contain exactly 77 records"',
        ".endif",
        ".global gAbilityPrefixEnd",
        "gAbilityPrefixEnd:",
        ".size gAbilityPrefix, gAbilityPrefixEnd - gAbilityPrefix",
    ]
    return "\n".join(lines) + "\n"


def extract_usa_prefix(rom: bytes) -> bytes:
    if len(rom) != 0x1000000 or hashlib.sha1(rom).hexdigest() != USA_SHA1:
        raise ValueError("input is not the verified USA baserom (expected SHA-1 " + USA_SHA1 + ")")
    return rom[START:START + SIZE]


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    raw = extract_usa_prefix(args.rom.read_bytes())
    args.output.write_text(render_asm(decode_table(raw)))


if __name__ == "__main__":
    main()
