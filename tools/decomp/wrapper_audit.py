#!/usr/bin/env python3
"""Audit the reserved ``asm/rom*.s`` copied-wrapper namespace.

This read-only report compares recognized copied-ROM wrappers with the copied
sources in the reviewed region ledger and sources resolved from Makefile OBJS.
It never removes or edits files. A malformed ledger or unsupported active
object/source mapping fails closed, without classifying files as orphans.
"""
import argparse
import importlib.util
import json
from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[2]
_progress_spec = importlib.util.spec_from_file_location(
    "ffta_progress_helpers", Path(__file__).with_name("progress.py"))
_progress = importlib.util.module_from_spec(_progress_spec)
_progress_spec.loader.exec_module(_progress)

_LABEL = re.compile(r"[A-Za-z_.$][A-Za-z0-9_.$]*:")
_GLOBAL = re.compile(r"\.global\s+[A-Za-z_.$][A-Za-z0-9_.$]*")
_SECTION = re.compile(r"\.section\s+\.text(?:\s*,\s*\"?\w+\"?)?")
_ALIGN = re.compile(r"\.(?:balign|align)\s+(?:0[xX][0-9a-fA-F]+|\d+)(?:\s*,\s*[^\s]+)?")


def _strip_comment(line):
    # GNU as uses @ for ARM assembly comments; generated wrappers also use it.
    return line.split("@", 1)[0].strip()


def _wrapper_kind(path, display_path=None):
    """Return empty/nonempty for the narrow generated wrapper form, else None.

    A zero-whitespace ``rom*.s`` member belongs to the reserved namespace by
    filename alone (the case which catches accidentally unlinked empty files).
    Content-bearing files count only when every meaningful line is a recognized
    baserom incbin or simple wrapper scaffolding. Authored instructions and
    unrelated assembly are deliberately outside the audit.
    """
    text = path.read_text()
    if not text.strip():
        return "empty"
    incbin_count = 0
    for raw in text.splitlines():
        line = _strip_comment(raw)
        if not line:
            continue
        if _progress.INCBIN.fullmatch(line):
            incbin_count += 1
            continue
        if (_SECTION.fullmatch(line) or _GLOBAL.fullmatch(line)
                or _ALIGN.fullmatch(line) or _LABEL.fullmatch(line)):
            continue
        # A malformed or foreign incbin inside this reserved-looking namespace
        # is ambiguous. Fail closed rather than treating it as authored source.
        if re.match(r"\.incbin\b", line):
            raise ValueError(f"unrecognized ROM wrapper incbin in {display_path or path.as_posix()}")
        return None
    return "nonempty" if incbin_count else None


def _resolve_active_sources(root):
    objects = _progress.parse_build_objects((root / "Makefile").read_text())
    sources = {}
    missing = []
    for obj in objects:
        stem = Path(obj).stem
        candidates = [f"{folder}/{stem}{suffix}"
                      for folder, suffix in (("asm", ".s"), ("data", ".s"), ("src", ".c"))]
        found = [source for source in candidates if (root / source).is_file()]
        if not found:
            missing.append({"object": obj, "expected_sources": candidates})
        elif len(found) != 1:
            raise ValueError(f"cannot resolve a unique source for {obj}: {', '.join(found)}")
        else:
            sources[obj] = found[0]
    return sources, missing


def audit(root):
    """Return a deterministic audit report for a repository-shaped root."""
    root = Path(root).resolve()
    try:
        manifest = json.loads((root / "config/decomp_regions.json").read_text())
    except json.JSONDecodeError as error:
        raise ValueError(f"invalid config/decomp_regions.json: {error}") from None
    # Validate the complete partition before reading any wrapper candidates.
    # In particular, do not let a broken ledger turn every filename into an
    # apparent orphan.
    _progress.measure(manifest)
    if not isinstance(manifest.get("regions"), list):
        raise ValueError("ledger regions must be a list")
    copied_sources = set()
    for region in manifest["regions"]:
        if region.get("kind") == "copied":
            source = region.get("source")
            # Other copied sources (for example the cartridge header) are
            # outside this intentionally narrow namespace.
            if isinstance(source, str) and re.fullmatch(r"asm/rom[^/]*\.s", source):
                copied_sources.add(source)

    active_by_object, missing_active = _resolve_active_sources(root)
    active_sources = set(active_by_object.values())
    missing_active.extend({"source": source, "reason": "copied ledger source is missing from active OBJS"}
                          for source in sorted(copied_sources - active_sources))
    active_wrappers_without_ledger = []
    for source in sorted(active_sources):
        if not re.fullmatch(r"asm/rom[^/]*\.s", source):
            continue
        kind = _wrapper_kind(root / source, source)
        if kind and source not in copied_sources:
            active_wrappers_without_ledger.append(source)

    # Missing active sources or ledger/build disagreement invalidate the source
    # authority. Return the defects, but deliberately produce no orphan verdict.
    if missing_active or active_wrappers_without_ledger:
        return {
            "schema": "ffta-wrapper-audit-v1", "ok": False,
            "errors": [], "missing_active_sources": missing_active,
            "active_wrappers_missing_ledger": active_wrappers_without_ledger,
            "empty_orphans": [], "nonempty_orphans": [],
            "orphan_count": 0,
        }

    wrapper_files = []
    for path in sorted((root / "asm").glob("rom*.s")):
        source = path.relative_to(root).as_posix()
        kind = _wrapper_kind(path, source)
        if kind:
            wrapper_files.append((source, kind))
    orphan_sources = {source for source, _ in wrapper_files} - copied_sources - active_sources
    empty = sorted(source for source, kind in wrapper_files
                   if source in orphan_sources and kind == "empty")
    nonempty = sorted(source for source, kind in wrapper_files
                      if source in orphan_sources and kind == "nonempty")
    return {
        "schema": "ffta-wrapper-audit-v1", "ok": not (empty or nonempty),
        "errors": [], "missing_active_sources": [],
        "active_wrappers_missing_ledger": [],
        "empty_orphans": empty, "nonempty_orphans": nonempty,
        "orphan_count": len(empty) + len(nonempty),
    }


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT,
                        help="repository root (defaults to this checkout)")
    parser.add_argument("--json", action="store_true", help="emit machine-readable JSON")
    args = parser.parse_args(argv)
    try:
        report = audit(args.root)
    except (OSError, KeyError, TypeError, ValueError) as error:
        if args.json:
            print(json.dumps({"schema": "ffta-wrapper-audit-v1", "ok": False,
                              "errors": [str(error)], "missing_active_sources": [],
                              "active_wrappers_missing_ledger": [],
                              "empty_orphans": [], "nonempty_orphans": [],
                              "orphan_count": 0}, sort_keys=True))
        else:
            print(f"Wrapper audit failed: {error}", file=sys.stderr)
        return 2
    if args.json:
        print(json.dumps(report, sort_keys=True, indent=2))
    else:
        if report["missing_active_sources"]:
            print("Missing active sources:")
            for item in report["missing_active_sources"]:
                print(f"  {item}")
        if report["active_wrappers_missing_ledger"]:
            print("Active copied wrappers missing from the ledger:")
            for source in report["active_wrappers_missing_ledger"]:
                print(f"  {source}")
        if report["empty_orphans"]:
            print("Empty orphan wrappers:")
            for source in report["empty_orphans"]:
                print(f"  {source} (remove after confirming no consumers)")
        if report["nonempty_orphans"]:
            print("Nonempty orphan wrappers:")
            for source in report["nonempty_orphans"]:
                print(f"  {source} (remove after confirming no consumers)")
        if report["ok"]:
            print("No orphan copied-ROM wrappers found.")
    return 0 if report["ok"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
