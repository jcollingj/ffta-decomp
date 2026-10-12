#!/usr/bin/env python3
"""Plan a copied-ROM region carve without changing repository files.

The plan is advisory. It does not edit the ROM, source, manifest, or Makefile,
and it does not establish that a proposed C implementation matches the game.
"""
import argparse
import importlib.util
import json
from pathlib import Path, PurePosixPath
import re


ROOT = Path(__file__).resolve().parents[2]
_progress_spec = importlib.util.spec_from_file_location(
    "ffta_progress_helpers", Path(__file__).with_name("progress.py"))
_progress = importlib.util.module_from_spec(_progress_spec)
_progress_spec.loader.exec_module(_progress)


def _source_path(value, label, root, allowed_dirs=None, allowed_suffixes=None):
    if not isinstance(value, str):
        raise ValueError(f"invalid {label} source path: {value}")
    path = PurePosixPath(value)
    if (path.is_absolute() or ".." in path.parts or len(path.parts) != 2
            or path.as_posix() != value
            or path.parts[0] not in (allowed_dirs or {"src", "asm", "data"})
            or path.suffix not in (allowed_suffixes or {".c", ".s"})
            or not re.fullmatch(r"[A-Za-z0-9_.+-]+", path.stem)
            or path.stem in {".", ".."}):
        raise ValueError(f"invalid {label} source path: {value}")
    try:
        (root / path.as_posix()).resolve().relative_to(root)
    except ValueError:
        raise ValueError(f"{label} source path escapes repository root: {value}") from None
    return path.as_posix()


def _source_object(source):
    return f"build/{Path(source).stem}.o"


def _resolve_objects(root, objects):
    mapping = {}
    for obj in objects:
        stem = Path(obj).stem
        matches = [f"{folder}/{stem}{suffix}"
                   for folder, suffix in (("asm", ".s"), ("data", ".s"), ("src", ".c"))
                   if (root / folder / f"{stem}{suffix}").is_file()]
        if len(matches) != 1:
            raise ValueError(f"cannot resolve a unique source for {obj}")
        mapping[obj] = matches[0]
    return mapping


def _incbins(root, source):
    text = (root / source).read_text()
    matches = list(_progress.INCBIN.finditer(text))
    if len(matches) != len(re.findall(r"^\s*\.incbin\b", text, re.M)):
        raise ValueError(f"unrecognized incbin in {source}")
    return [(int(m.group(1), 0), int(m.group(2), 0)) for m in matches]


def plan(root, start, end, source, prefix=None, suffix=None):
    """Return a deterministic carve plan for ROM offsets [start, end)."""
    root = Path(root).resolve()
    if not isinstance(start, int) or not isinstance(end, int):
        raise ValueError("start and end must be integer ROM offsets")
    manifest = json.loads((root / "config/decomp_regions.json").read_text())
    _progress.measure(manifest)
    size = manifest.get("rom_size")
    if start < 0 or end <= start or end > size:
        raise ValueError("carve interval must be nonempty and inside ROM bounds")
    if start % 4 or end % 4:
        raise ValueError("carve boundaries must be 4-byte aligned")

    copied = [r for r in manifest["regions"]
              if r.get("kind") == "copied" and r["start"] <= start and end <= r["end"]]
    overlaps = [r for r in manifest["regions"]
                if r["kind"] != "copied" and r["start"] < end and start < r["end"]]
    if len(copied) != 1 or overlaps:
        raise ValueError("interval must lie in exactly one copied region and overlap no recovered region")
    old = copied[0]
    old_start, old_end = old["start"], old["end"]
    old_source = _source_path(old["source"], "old copied", root,
                              allowed_dirs={"asm", "data"}, allowed_suffixes={".s"})
    if not (root / old_source).is_file():
        raise ValueError(f"missing copied source: {old_source}")

    objects = _progress.parse_build_objects((root / "Makefile").read_text())
    object_sources = _resolve_objects(root, objects)
    old_object = _source_object(old_source)
    if objects.count(old_object) != 1 or object_sources.get(old_object) != old_source:
        raise ValueError("copied source object must occur exactly once in OBJS")
    if _incbins(root, old_source) != [(old_start, old_end - old_start)]:
        raise ValueError("copied source incbin must exactly match its ledger interval")

    source = _source_path(source, "new", root, {"src"}, {".c"})
    if not source.startswith("src/") or not source.endswith(".c"):
        raise ValueError("new source must be src/NAME.c")

    segments = []
    if old_start < start:
        segments.append(("prefix", old_start, start))
    if end < old_end:
        segments.append(("suffix", end, old_end))
    requested = {"prefix": prefix, "suffix": suffix}
    remainder_sources = {}
    old_path = PurePosixPath(old_source)
    for label, seg_start, seg_end in segments:
        candidate = requested[label]
        if candidate is None:
            candidate = f"{old_path.parent}/{old_path.stem}_{label}.s"
        candidate = _source_path(candidate, label, root,
                                  {"asm", "data"}, {".s"})
        if candidate == source or (root / candidate).exists():
            raise ValueError(f"{label} source path already exists or collides: {candidate}")
        remainder_sources[label] = candidate

    for label, candidate in (("prefix", prefix), ("suffix", suffix)):
        if candidate is not None and label not in remainder_sources:
            raise ValueError(f"{label} filename supplied but carve has no {label} remainder")
    proposed_sources = [source, *remainder_sources.values()]
    if len({_source_object(p) for p in proposed_sources}) != len(proposed_sources):
        raise ValueError("new source/object filenames collide")
    if (root / source).exists():
        raise ValueError(f"new source path already exists: {source}")

    new_objects = []
    if "prefix" in remainder_sources:
        new_objects.append(_source_object(remainder_sources["prefix"]))
    new_objects.append(_source_object(source))
    if "suffix" in remainder_sources:
        new_objects.append(_source_object(remainder_sources["suffix"]))
    if set(new_objects) & set(objects):
        raise ValueError("proposed object name collides with an existing OBJS object")
    for proposed_source in proposed_sources:
        stem = Path(proposed_source).stem
        if any((root / folder / f"{stem}{suffix}").exists()
               for folder, suffix in (("asm", ".s"), ("data", ".s"), ("src", ".c"))):
            raise ValueError(f"proposed source stem collides with an existing source: {stem}")
    insertion = objects.index(old_object)
    proposed_objects = objects[:insertion] + new_objects + objects[insertion + 1:]
    unrelated_before = [obj for obj in objects if obj != old_object]
    unrelated_after = [obj for obj in proposed_objects if obj not in new_objects]
    if unrelated_before != unrelated_after:
        raise AssertionError("internal error: unrelated objects changed order")

    remainder_records = []
    for label, seg_start, seg_end in segments:
        remainder_records.append({
            "side": label,
            "start": seg_start,
            "end": seg_end,
            "source": remainder_sources[label],
            "object": _source_object(remainder_sources[label]),
            "incbin": {"offset": seg_start, "length": seg_end - seg_start},
        })
    return {
        "schema": "ffta-carve-plan-v1",
        "read_only": True,
        "matching_c_claim": False,
        "old_interval": {"start": old_start, "end": old_end,
                         "length": old_end - old_start,
                         "source": old_source, "object": old_object},
        "carve": {"start": start, "end": end, "length": end - start,
                  "source": source, "object": _source_object(source)},
        "remainders": remainder_records,
        "objects": {"before": objects, "after": proposed_objects,
                    "unrelated_preserved_in_order": unrelated_before},
        "files": {
            "create": sorted([source, *remainder_sources.values()]),
            "edit": sorted(["Makefile", "config/decomp_regions.json", "ld/rom.ld"]),
            "retire": [{"path": old_source,
                        "when": "after replacement and remainder sources are integrated",
                        "reason": "the old copied object is removed from OBJS"}],
        },
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--start", required=True, type=lambda x: int(x, 0),
                        help="inclusive ROM offset (decimal or 0x-prefixed)")
    parser.add_argument("--end", required=True, type=lambda x: int(x, 0),
                        help="exclusive ROM offset (decimal or 0x-prefixed)")
    parser.add_argument("--source", required=True, help="new src/NAME.c path")
    parser.add_argument("--prefix", help="optional prefix source path, for example asm/rom_x_prefix.s")
    parser.add_argument("--suffix", help="optional suffix source path, for example asm/rom_x_suffix.s")
    parser.add_argument("--json", action="store_true", help="emit deterministic machine-readable JSON")
    args = parser.parse_args()
    try:
        result = plan(args.root, args.start, args.end, args.source, args.prefix, args.suffix)
    except (OSError, KeyError, TypeError, json.JSONDecodeError, ValueError) as error:
        parser.error(str(error))
    if args.json:
        print(json.dumps(result, sort_keys=True, indent=2))
    else:
        print(f"Carve plan: [{result['carve']['start']:#x}, {result['carve']['end']:#x})")
        print(f"Old copied interval: [{result['old_interval']['start']:#x}, {result['old_interval']['end']:#x})")
        print("OBJS after carve:")
        print("\n".join(result["objects"]["after"]))
        print(f"Create: {', '.join(result['files']['create'])}")
        print(f"Edit: {', '.join(result['files']['edit'])}")
        print("Read-only plan; no matching-C claim.")


if __name__ == "__main__":
    main()
