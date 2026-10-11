#!/usr/bin/env python3
"""Build Field Aid: recover full HP, clear debuffs, and grant Regen."""
import argparse
import hashlib
import json
from pathlib import Path
import rename_ability


def patch(data, spec):
    result, name_report = rename_ability.patch(data, spec)
    offset = rename_ability.ABILITIES + spec["ability_id"] * 28
    expected = bytes.fromhex(spec["expected_record"])
    if len(expected) != 28 or data[offset:offset + 28] != expected:
        raise ValueError("original ability record does not match")
    if spec["effects"] != [0x59, 0x26, 0xC3, 1]:
        raise ValueError("only the verified full-heal / cleanse / Regen combination is supported")
    result = bytearray(result)
    result[offset + 12:offset + 16] = bytes(spec["effects"])
    changes = [{"offset": hex(i), "before": a, "after": b}
               for i, (a, b) in enumerate(zip(data, result)) if a != b]
    return bytes(result), {
        "spec": spec, "name_patch": name_report, "record_offset": hex(offset),
        "original_record": expected.hex(), "modified_record": result[offset:offset + 28].hex(),
        "input_sha1": rename_ability.BASE_SHA1,
        "output_sha1": hashlib.sha1(result).hexdigest(), "changes": changes,
    }


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--input", type=Path, default=Path("build/ffta.gba"))
    p.add_argument("--spec", type=Path, default=Path("mods/field-aid.json"))
    p.add_argument("--output", type=Path, default=Path("build/mods/field-aid/ffta.gba"))
    a = p.parse_args()
    if a.input.resolve() == a.output.resolve():
        p.error("output must be separate from input")
    result, report = patch(a.input.read_bytes(), json.loads(a.spec.read_text()))
    a.output.parent.mkdir(parents=True, exist_ok=True)
    a.output.write_bytes(result)
    a.output.with_suffix(".manifest.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
