#!/usr/bin/env python3
"""Render a validated copied-ROM carve plan as a unified patch.

This tool writes nothing. The emitted patch only adds copied incbin wrappers,
retires the superseded wrapper, and replaces the Makefile OBJS assignment.
"""
import argparse
import difflib
import importlib.util
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[2]
_spec = importlib.util.spec_from_file_location(
    "ffta_carve_plan", Path(__file__).with_name("carve_plan.py"))
_carve_plan = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_carve_plan)


def _wrapper(record):
    offset = record["incbin"]["offset"]
    length = record["incbin"]["length"]
    return ("@ Unrecovered original code/data retained around a source carve.\n"
            ".section .text\n"
            f'.incbin "baserom.gba", {offset:#x}, {length:#x}\n')


def _makefile_after(makefile, objects):
    lines = makefile.splitlines(keepends=True)
    assignments = [i for i, line in enumerate(lines)
                   if re.match(r"^\s*OBJS\b", line)]
    if len(assignments) != 1:
        raise ValueError("expected exactly one explicit OBJS assignment")
    start = assignments[0]
    end = start
    while lines[end].rstrip("\r\n").rstrip().endswith("\\"):
        end += 1
        if end >= len(lines):
            raise ValueError("unterminated OBJS continuation")
    block = lines[start:end + 1]
    old_objects = _carve_plan._progress.parse_build_objects("".join(block))
    old_obj = next((obj for obj in old_objects if obj not in objects), None)
    if old_obj is None:
        raise ValueError("cannot identify the retired object in OBJS")
    target = [i for i in range(start, end + 1) if old_obj in lines[i].split()]
    if len(target) != 1:
        raise ValueError("unsupported OBJS layout for replacing the copied object")
    index = target[0]
    line = lines[index]
    body, comment = (line.split("#", 1) + [None])[:2] if "#" in line else (line, None)
    match = re.match(r"^(\s*)(.*?)(\\?)(\r?\n)?$", body)
    indent, content, continuation, newline = match.groups()
    tokens = content.split()
    if tokens != [old_obj]:
        raise ValueError("unsupported OBJS layout: copied object must be alone on its line")
    replacements = [obj for obj in objects if obj not in old_objects]
    if not replacements:
        raise ValueError("no replacement objects in proposed OBJS sequence")
    ending = newline or ""
    new_lines = []
    for position, obj in enumerate(replacements):
        is_last = position == len(replacements) - 1
        slash = continuation if is_last else "\\"
        suffix = (" #" + comment) if comment is not None and is_last else ""
        line_ending = ending or ("\n" if not is_last else "")
        new_lines.append(f"{indent}{obj}{(' ' + slash) if slash else ''}{suffix}{line_ending}")
    return "".join(lines[:index] + new_lines + lines[index + 1:])


def _diff(path, before, after):
    old_name = "/dev/null" if not before else f"a/{path}"
    new_name = "/dev/null" if not after else f"b/{path}"
    metadata = f"diff --git a/{path} b/{path}\n"
    if not before:
        metadata += "new file mode 100644\n"
    elif not after:
        metadata += "deleted file mode 100644\n"
    old_lines = before.splitlines(keepends=True)
    new_lines = after.splitlines(keepends=True)
    old_missing_eol = bool(old_lines) and not old_lines[-1].endswith("\n")
    new_missing_eol = bool(new_lines) and not new_lines[-1].endswith("\n")
    # difflib does not emit Git's no-final-newline marker. Normalize physical
    # diff rows, then restore the marker on any EOF row included in a hunk.
    if old_missing_eol:
        old_lines[-1] += "\n"
    if new_missing_eol:
        new_lines[-1] += "\n"
    rows = difflib.unified_diff(old_lines, new_lines, fromfile=old_name, tofile=new_name)
    output = []
    old_cursor = new_cursor = None
    old_count, new_count = len(before.splitlines()), len(after.splitlines())
    for row in rows:
        hunk = re.match(r"^@@ -(\d+)(?:,\d+)? \+(\d+)(?:,\d+)? @@", row)
        if hunk:
            old_cursor, new_cursor = (int(value) for value in hunk.groups())
        output.append(row)
        missing = False
        if row.startswith(" ") and old_cursor is not None:
            missing = (old_missing_eol and new_missing_eol
                       and old_cursor == old_count and new_cursor == new_count)
            old_cursor += 1
            new_cursor += 1
        elif row.startswith("-") and not row.startswith("---") and old_cursor is not None:
            missing = old_missing_eol and old_cursor == old_count
            old_cursor += 1
        elif row.startswith("+") and not row.startswith("+++") and new_cursor is not None:
            missing = new_missing_eol and new_cursor == new_count
            new_cursor += 1
        if missing:
            output.append("\\ No newline at end of file\n")
    return metadata + "".join(output)


def render(root, start, end, source, prefix=None, suffix=None):
    """Return a unified patch for the currently validated plan, without writes."""
    root = Path(root).resolve()
    plan = _carve_plan.plan(root, start, end, source, prefix, suffix)
    changes = []
    for remainder in plan["remainders"]:
        changes.append((remainder["source"], "", _wrapper(remainder)))
    old_path = plan["old_interval"]["source"]
    old_text = (root / old_path).read_text()
    changes.append((old_path, old_text, ""))
    makefile = (root / "Makefile").read_text()
    changes.append(("Makefile", makefile,
                    _makefile_after(makefile, plan["objects"]["after"])))
    patch = []
    for path, before, after in changes:
        patch.append(_diff(path, before, after))
    return "".join(patch)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--start", required=True, type=lambda value: int(value, 0),
                        help="inclusive ROM offset (decimal or 0x-prefixed)")
    parser.add_argument("--end", required=True, type=lambda value: int(value, 0),
                        help="exclusive ROM offset (decimal or 0x-prefixed)")
    parser.add_argument("--source", required=True, help="new src/NAME.c path")
    parser.add_argument("--prefix", help="optional prefix source path")
    parser.add_argument("--suffix", help="optional suffix source path")
    args = parser.parse_args(argv)
    try:
        sys.stdout.write(render(args.root, args.start, args.end, args.source,
                                args.prefix, args.suffix))
    except (OSError, KeyError, TypeError, ValueError) as error:
        parser.error(str(error))


if __name__ == "__main__":
    main()
