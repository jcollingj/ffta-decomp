#!/usr/bin/env python3
"""Build a bounded ability-name edit from a verified matching USA ROM."""
import argparse
import hashlib
import json
from pathlib import Path
import struct

BASE_SHA1 = "4ac05441f4de70a4ec3dd932116346c61b8783d9"
NAMES = 0x5567F0
ABILITIES = 0x55187C


def encode(text, compact):
    """Encode only the alphabet/space subset required by this experiment."""
    out = bytearray(b"\x01" if compact else b"")
    for c in text:
        if "A" <= c <= "Z":
            value = ord(c) - ord("A") + 0xB0
        elif "a" <= c <= "z":
            value = ord(c) - ord("a") + 0xCA
        elif c == " " and not compact:
            out.extend(b"\x40\x73")
            continue
        else:
            raise ValueError(f"unsupported character {c!r} for this encoding")
        out.extend(bytes([value + 1]) if compact else bytes([0x80, value]))
    return bytes(out) + b"\x00"


def patch(data, spec):
    if len(data) != 0x1000000 or hashlib.sha1(data).hexdigest() != BASE_SHA1:
        raise ValueError("input must be the exact matching FFTA USA ROM")
    ability, string = spec["ability_id"], spec["string_id"]
    if not isinstance(ability, int) or not 0 <= ability < 346:
        raise ValueError("invalid ability ID")
    if not isinstance(string, int) or not 0 <= string < 767:
        raise ValueError("invalid string ID")
    record = ABILITIES + ability * 0x1C
    if struct.unpack_from("<H", data, record)[0] != string:
        raise ValueError("ability does not reference the specified string")
    pointer = NAMES + string * 4
    offset = struct.unpack_from("<I", data, pointer)[0] - 0x08000000
    if not 0 <= offset < len(data):
        raise ValueError("invalid ROM string pointer")
    compact = data[offset] == 1
    original = encode(spec["expected_name"], compact)
    replacement = encode(spec["replacement_name"], compact)
    if data[offset:offset + len(original)] != original:
        raise ValueError("expected original name bytes do not match")
    if len(replacement) != len(original):
        raise ValueError("this experiment requires identical encoded length")
    result = bytearray(data)
    result[offset:offset + len(original)] = replacement
    report = dict(spec, string_offset=hex(offset), pointer_offset=hex(pointer),
                  original_bytes=original.hex(), replacement_bytes=replacement.hex(),
                  input_sha1=BASE_SHA1, output_sha1=hashlib.sha1(result).hexdigest(),
                  changed_bytes=sum(a != b for a, b in zip(data, result)))
    return bytes(result), report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path, default=Path("build/ffta.gba"))
    parser.add_argument("--spec", type=Path, default=Path("mods/ability-name.json"))
    parser.add_argument("--output", type=Path, default=Path("build/mods/ability-name/ffta.gba"))
    args = parser.parse_args()
    if args.input.resolve() == args.output.resolve():
        parser.error("output must be separate from input")
    result, report = patch(args.input.read_bytes(), json.loads(args.spec.read_text()))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(result)
    args.output.with_suffix(".manifest.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
