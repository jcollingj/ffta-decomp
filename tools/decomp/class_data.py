"""Lossless decoder/encoder for the evidenced class-record prefixes."""
from __future__ import annotations

import argparse
from pathlib import Path


CLASS4_START = 0x39475C
CLASS4_RECORD_SIZE = 4
CLASS4_COUNT = 7
CLASS16_START = 0x528F34
CLASS16_RECORD_SIZE = 16
# ReadCollectionClass maps its 16-byte lookup indices into 0..85. A separate
# reader also uses this stride for indices through 85. Later bytes up to the
# next known symbol have not been classified as this table.
CLASS16_COUNT = 86

CLASS4_FIELDS = {
    "field00": (0, 1, "u8"),
    "unknown01": (1, 1, "u8"),
    "unknown02": (2, 1, "u8"),
    "unknown03": (3, 1, "u8"),
}
CLASS16_FIELDS = {
    "field00": (0, 1, "u8"),
    "unknown01": (1, 1, "u8"),
    "field02": (2, 2, "u16"),
    "unknown04": (4, 12, "bytes"),
}


def _decode_record(data: bytes, size: int, fields: dict) -> dict:
    if len(data) != size:
        raise ValueError(f"record must be exactly {size} bytes")
    result = {}
    for name, (offset, width, kind) in fields.items():
        chunk = data[offset:offset + width]
        result[name] = chunk.hex() if kind == "bytes" else int.from_bytes(chunk, "little")
    return result


def _range(name: str, value: object, bits: int) -> None:
    if type(value) is not int or not 0 <= value < (1 << bits):
        raise ValueError(f"{name} must fit unsigned {bits}-bit field")


def _encode_record(record: dict, size: int, fields: dict) -> bytes:
    if not isinstance(record, dict) or set(record) != set(fields):
        raise ValueError("record fields do not match the documented schema")
    out = bytearray(size)
    for name, (offset, width, kind) in fields.items():
        value = record[name]
        if kind == "bytes":
            if not isinstance(value, str) or len(value) != width * 2:
                raise ValueError(f"{name} must be exactly {width} raw bytes as hex")
            try:
                raw = bytes.fromhex(value)
            except ValueError as error:
                raise ValueError(f"{name} must be hexadecimal") from error
            if len(raw) != width:
                raise ValueError(f"{name} must decode to exactly {width} bytes")
            out[offset:offset + width] = raw
        else:
            bits = width * 8
            _range(name, value, bits)
            out[offset:offset + width] = value.to_bytes(width, "little")
    return bytes(out)


def decode_class4_record(data: bytes) -> dict:
    return _decode_record(data, CLASS4_RECORD_SIZE, CLASS4_FIELDS)


def encode_class4_record(record: dict) -> bytes:
    return _encode_record(record, CLASS4_RECORD_SIZE, CLASS4_FIELDS)


def decode_class16_record(data: bytes) -> dict:
    return _decode_record(data, CLASS16_RECORD_SIZE, CLASS16_FIELDS)


def encode_class16_record(record: dict) -> bytes:
    return _encode_record(record, CLASS16_RECORD_SIZE, CLASS16_FIELDS)


def _decode_table(data: bytes, count: int, size: int, decoder) -> list[dict]:
    expected = count * size
    if len(data) != expected:
        raise ValueError(f"table must be exactly {expected} bytes ({count} records)")
    return [decoder(data[offset:offset + size]) for offset in range(0, expected, size)]


def decode_class4_table(data: bytes) -> list[dict]:
    return _decode_table(data, CLASS4_COUNT, CLASS4_RECORD_SIZE, decode_class4_record)


def decode_class16_table(data: bytes) -> list[dict]:
    return _decode_table(data, CLASS16_COUNT, CLASS16_RECORD_SIZE, decode_class16_record)


def encode_class4_table(records: list[dict]) -> bytes:
    if len(records) != CLASS4_COUNT:
        raise ValueError(f"class4 table must contain {CLASS4_COUNT} records")
    return b"".join(encode_class4_record(record) for record in records)


def encode_class16_table(records: list[dict]) -> bytes:
    if len(records) != CLASS16_COUNT:
        raise ValueError(f"class16 table must contain {CLASS16_COUNT} records")
    return b"".join(encode_class16_record(record) for record in records)


def _macro_fields(fields: dict) -> list[str]:
    names = []
    for name, (_, width, kind) in fields.items():
        names.extend(f"{name}_{index:02d}" for index in range(width) if kind == "bytes")
        if kind != "bytes":
            names.append(name)
    return names


def _render_table(records: list[dict], *, symbol: str, macro: str, fields: dict,
                  count: int, size: int, comments: tuple[str, ...]) -> str:
    if len(records) != count:
        raise ValueError(f"{symbol} must contain exactly {count} evidenced records")
    encoded = b"".join(_encode_record(record, size, fields) for record in records)
    del encoded  # Validate all values before emitting assembler.
    lines = [*comments, ".section .text", ".balign 4", f".global {symbol}",
             f".type {symbol}, %object"]
    params = _macro_fields(fields)
    lines += [
        ".macro class_u8 value",
        "    .if (\\value < 0) || (\\value > 255)",
        '        .error "class byte out of range"',
        "    .endif",
        "    .byte \\value",
        ".endm",
        ".macro class_u16 value",
        "    .if (\\value < 0) || (\\value > 65535)",
        '        .error "class halfword out of range"',
        "    .endif",
        "    .hword \\value",
        ".endm",
        f".macro {macro} id, " + ", ".join(params),
        f"    .if (. - {symbol}) != (\\id * {size})",
        f'        .error "{macro} IDs must be consecutive from zero"',
        "    .endif",
    ]
    for name, (_, width, kind) in fields.items():
        if kind == "bytes":
            lines.extend(f"    class_u8 \\{name}_{index:02d}" for index in range(width))
        elif kind == "u16":
            lines.append(f"    class_u16 \\{name}")
        else:
            lines.append(f"    class_u8 \\{name}")
    lines.extend([".endm", f"{symbol}:"])
    for index, record in enumerate(records):
        values = []
        for name, (_, width, kind) in fields.items():
            value = record[name]
            values.extend(bytes.fromhex(value) if kind == "bytes" else [value])
        lines.append(f"    {macro} {index}, " + ", ".join(map(str, values)))
    lines.extend([
        f".if (. - {symbol}) != {count * size}",
        f'    .error "{symbol} table size changed"',
        ".endif",
        f".size {symbol}, .-{symbol}",
    ])
    return "\n".join(lines) + "\n"


def render_class4_asm(records: list[dict]) -> str:
    return _render_table(
        records, symbol="gClassRecords4", macro="class4_record", fields=CLASS4_FIELDS,
        count=CLASS4_COUNT, size=CLASS4_RECORD_SIZE,
        comments=("@ Bounded 7-record prefix at ROM 0x0839475C.",
                  "@ Field names reflect observed offsets only."),
    )


def render_class16_asm(records: list[dict]) -> str:
    return _render_table(
        records, symbol="gClassRecords16", macro="class16_record", fields=CLASS16_FIELDS,
        count=CLASS16_COUNT, size=CLASS16_RECORD_SIZE,
        comments=("@ Bounded 86-record prefix at ROM 0x08528F34.",
                  "@ Records beyond index 85 remain unclassified/copied."),
    )


def decode_rom(rom: bytes) -> tuple[list[dict], list[dict]]:
    end4 = CLASS4_START + CLASS4_COUNT * CLASS4_RECORD_SIZE
    end16 = CLASS16_START + CLASS16_COUNT * CLASS16_RECORD_SIZE
    if len(rom) < max(end4, end16):
        raise ValueError("input ROM is truncated before an evidenced class table")
    return (
        decode_class4_table(rom[CLASS4_START:end4]),
        decode_class16_table(rom[CLASS16_START:end16]),
    )


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path)
    parser.add_argument("output_dir", type=Path)
    args = parser.parse_args()
    class4, class16 = decode_rom(args.rom.read_bytes())
    args.output_dir.mkdir(parents=True, exist_ok=True)
    (args.output_dir / "class_records4.s").write_text(render_class4_asm(class4))
    (args.output_dir / "class_records16.s").write_text(render_class16_asm(class16))


if __name__ == "__main__":
    main()
