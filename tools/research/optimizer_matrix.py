#!/usr/bin/env python3
"""Compare compiler optimization levels on a fixed set of complete C TUs.

This is diagnostic research only. It does not change production flags or admit
functions into the recovered build.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
from typing import Any

ROOT = Path(__file__).resolve().parents[2]
REFERENCE = Path("build/verified-278-tree-layout-ref.elf")
REFERENCE_SHA256 = "be0a3ca7edfc3057225e11b125451376d250ea2b3c7cd52d0953a02276fbc2e4"
COMPILER_BINARY = Path("tools/.deps/agbcc/gcc/agbcc")
OPTS = ("0", "1", "2", "s")

# These are verified trailing zero-alignment extents beyond the manifest's
# function-symbol bounds. Function starts/ends themselves always come from the
# manifests; only these observed block suffixes are explicit.
BLOCK_SUFFIXES = {
    "src/nonmatching/tree_detach.c": {"DetachTreeNode": 2},
    "src/nonmatching/tree_root_count.c": {"CountTreeRootNodes": 2},
    "src/list_callbacks.c": {"ListCallbackClearFlag0": 2},
    "src/list_append.c": {"AppendListNode": 2},
}

SOURCES = (
    ("candidate", "src/nonmatching/content_flag_clear.c", ("ClearContentFlag21FIfSet",)),
    ("candidate", "src/nonmatching/tree_insert.c", ("InsertTreeNode",)),
    ("candidate", "src/nonmatching/tree_detach.c", ("DetachTreeNode",)),
    ("candidate", "src/nonmatching/container_destroy.c", ("DestroyContainer",)),
    ("candidate", "src/nonmatching/tree_root_count.c", ("CountTreeRootNodes",)),
    ("matching", "src/list_callbacks.c", ("ListCallbackAppend", "ListCallbackSelectClear", "ListCallbackClearFlag0")),
    ("matching", "src/content_index.c", ("ReadCurrentContentIndex", "ReadContentIndexOrFallback")),
    ("matching", "src/list_append.c", ("AppendListNode",)),
)


class MatrixError(RuntimeError):
    pass


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def read_json(path: Path) -> Any:
    try:
        return json.loads(path.read_text())
    except (OSError, json.JSONDecodeError) as exc:
        raise MatrixError(f"cannot read valid JSON from {path}: {exc}") from exc


def descriptor_plan(root: Path = ROOT) -> list[dict[str, Any]]:
    regions = read_json(root / "config/decomp_regions.json").get("functions")
    candidates = read_json(root / "config/decomp_candidates.json").get("functions")
    if not isinstance(regions, list) or not isinstance(candidates, list):
        raise MatrixError("function manifests must contain function arrays")
    entries = {"matching": regions, "candidate": candidates}
    plan = []
    for kind, source, expected_names in SOURCES:
        path = root / source
        if not path.is_file():
            raise MatrixError(f"missing matrix source: {source}")
        matches = [item for item in entries[kind] if item.get("source") == source]
        if kind == "candidate":
            matches = [item for item in matches if item.get("name") in expected_names]
        if not matches:
            raise MatrixError(f"no {kind} manifest function for {source}")
        actual_names = {item.get("name") for item in matches}
        if actual_names != set(expected_names) or len(matches) != len(expected_names):
            raise MatrixError(f"manifest symbols for {source} differ from fixed matrix set: {sorted(actual_names)}")
        matches.sort(key=lambda item: (item.get("start", -1), item.get("name", "")))
        for item in matches:
            if item.get("mode") != "thumb" or not isinstance(item.get("start"), int) or not isinstance(item.get("end"), int):
                raise MatrixError(f"malformed bounds/mode for {item.get('name')} in {source}")
            if item["start"] >= item["end"]:
                raise MatrixError(f"empty/reversed bounds for {item['name']}")
        for left, right in zip(matches, matches[1:]):
            if left["end"] != right["start"]:
                raise MatrixError(f"noncontiguous symbols in complete TU {source}: {left['name']} / {right['name']}")
        suffix_map = BLOCK_SUFFIXES.get(source, {})
        if suffix_map and set(suffix_map) != {matches[-1]["name"]}:
            raise MatrixError(f"alignment suffix must identify only the final symbol in {source}")
        suffix = suffix_map.get(matches[-1]["name"], 0)
        block_start, symbol_end = matches[0]["start"], matches[-1]["end"]
        block_end = symbol_end + suffix
        if block_end & 1:
            raise MatrixError(f"unaligned probe block for {source}: {block_end:#x}")
        plan.append({
            "kind": kind,
            "source": source,
            "source_sha256": sha256_bytes(path.read_bytes()),
            "start": block_start,
            "start_hex": hex(block_start),
            "end": block_end,
            "end_hex": hex(block_end),
            "symbol_end": symbol_end,
            "symbols": [
                {"name": item["name"], "start": item["start"], "start_hex": hex(item["start"]),
                 "end": item["end"], "end_hex": hex(item["end"]),
                 "size": item["end"] - item["start"]}
                for item in matches
            ],
            "alignment_suffix": suffix,
        })
    if sum(item["kind"] == "candidate" for item in plan) < 3 or sum(item["kind"] == "matching" for item in plan) < 3:
        raise MatrixError("matrix requires at least three candidate and three matching translation units")
    return plan


def validate_reference(root: Path = ROOT) -> str:
    path = root / REFERENCE
    if not path.is_file():
        raise MatrixError(f"required immutable reference ELF is missing: {path}")
    digest = sha256_bytes(path.read_bytes())
    if digest != REFERENCE_SHA256:
        raise MatrixError(f"reference ELF hash mismatch: {digest}")
    return digest


def normalize_probe(report: Any, descriptor: dict[str, Any], opt: str) -> dict[str, Any]:
    if not isinstance(report, dict) or not isinstance(report.get("functions"), list):
        raise MatrixError("probe comparison JSON has no function list")
    if report.get("optimization") != opt:
        raise MatrixError("probe optimization does not match requested level")
    wanted = {item["name"]: item for item in descriptor["symbols"]}
    seen: dict[str, dict[str, Any]] = {}
    for fn in report["functions"]:
        if not isinstance(fn, dict) or fn.get("name") not in wanted:
            raise MatrixError(f"unexpected or malformed probe symbol record: {fn!r}")
        name = fn["name"]
        if name in seen:
            raise MatrixError(f"duplicate symbol record for {name}")
        required = ("status", "matches", "expected_size", "actual_size", "differing_bytes",
                    "reference_start", "actual_start", "address_delta", "layout_matches")
        if any(key not in fn for key in required):
            raise MatrixError(f"incomplete strict symbol metrics for {name}")
        if fn["expected_size"] != wanted[name]["size"] or fn["reference_start"] != wanted[name]["start"]:
            raise MatrixError(f"manifest symbol bounds disagree with probe record for {name}")
        integer_fields = ("expected_size", "actual_size", "reference_start", "actual_start", "address_delta", "differing_bytes")
        if (any(type(fn[key]) is not int for key in integer_fields) or
                type(fn["matches"]) is not bool or type(fn["layout_matches"]) is not bool or
                fn["differing_bytes"] < 0 or fn["actual_size"] < 0 or fn["expected_size"] < 0):
            raise MatrixError(f"invalid symbol metrics for {name}")
        if fn["address_delta"] != fn["actual_start"] - fn["reference_start"]:
            raise MatrixError(f"inconsistent address delta for {name}")
        if fn["matches"] != (fn["differing_bytes"] == 0 and fn["actual_size"] == fn["expected_size"]):
            raise MatrixError(f"inconsistent byte equality metrics for {name}")
        if (fn["layout_matches"] != (fn["actual_start"] == fn["reference_start"] and
                                     fn["actual_size"] == fn["expected_size"]) or
                fn["status"] != ("MATCH" if fn["matches"] and fn["layout_matches"] else "DIFF")):
            raise MatrixError(f"inconsistent strict symbol status/layout metrics for {name}")
        seen[name] = fn
    if set(seen) != set(wanted):
        raise MatrixError(f"probe symbols missing: {sorted(set(wanted) - set(seen))}")
    block_fields = ("matches", "expected_size", "actual_size", "differing_bytes")
    if any(field not in report for field in block_fields):
        raise MatrixError("probe comparison JSON lacks whole-block metrics")
    if (type(report["matches"]) is not bool or type(report["expected_size"]) is not int or
            type(report["actual_size"]) is not int or type(report["differing_bytes"]) is not int or
            report["expected_size"] < 0 or report["actual_size"] < 0 or report["differing_bytes"] < 0):
        raise MatrixError("probe comparison JSON has malformed whole-block metrics")
    if report["matches"] != (report["expected_size"] == report["actual_size"] and report["differing_bytes"] == 0):
        raise MatrixError("inconsistent whole-block equality metrics")
    if report["expected_size"] != descriptor["end"] - descriptor["start"]:
        raise MatrixError("whole-block expected extent differs from derived descriptor")
    rows = []
    for name in wanted:
        fn = seen[name]
        rows.append({key: fn[key] for key in (
            "name", "status", "matches", "expected_size", "actual_size", "differing_bytes",
            "reference_start", "actual_start", "address_delta", "layout_matches")})
    return {
        "optimization": opt,
        "symbols": rows,
        "block": {key: report[key] for key in block_fields},
    }


def run_matrix(root: Path = ROOT, keep_artifacts: bool = False) -> dict[str, Any]:
    reference_hash = validate_reference(root)
    compiler_path = root / COMPILER_BINARY
    if not compiler_path.is_file():
        raise MatrixError(f"pinned agbcc compiler binary is missing: {compiler_path}")
    compiler_hash = sha256_bytes(compiler_path.read_bytes())
    plan = descriptor_plan(root)
    toolchain = read_json(root / "config/toolchain.json")
    env = dict(os.environ)
    probe_root = root / "build/probes"
    probe_root.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="session62_optimizer_", dir=probe_root) as scratch_name:
        scratch = Path(scratch_name)
        outputs = []
        preprocessed_hashes: dict[str, str] = {}
        for desc_index, desc in enumerate(plan):
            original = root / desc["source"]
            source_bytes = original.read_bytes()
            if sha256_bytes(source_bytes) != desc["source_sha256"]:
                raise MatrixError(f"source changed after descriptor plan: {desc['source']}")
            for opt in OPTS:
                unique_stem = f"s62_{desc_index:02d}_{desc['source'].replace('/', '_').replace('.c', '')}_O{opt}_{desc['source_sha256'][:10]}"
                copied_source = scratch / f"{unique_stem}.c"
                copied_source.write_bytes(source_bytes)
                output_folder = root / "build/probes" / unique_stem
                if output_folder.exists():
                    raise MatrixError(f"probe output collision; refusing stale output: {output_folder}")
                command = [sys.executable, "tools/decomp/probe.py", str(copied_source),
                           "--start", hex(desc["start"]), "--end", hex(desc["end"]),
                           "--opt", opt, "--reference-elf", str(root / REFERENCE)]
                completed = subprocess.run(command, cwd=root, capture_output=True, text=True, env=env)
                report_path = output_folder / "comparison.json"
                if not report_path.is_file():
                    raise MatrixError(f"probe produced no fresh comparison.json for {unique_stem}; exit={completed.returncode}; stderr={completed.stderr[-1200:]}")
                if completed.returncode not in (0, 1):
                    raise MatrixError(f"probe subprocess failed for {unique_stem}: {completed.returncode}; {completed.stderr[-1200:]}")
                preprocessed_path = output_folder / "candidate.i"
                if not preprocessed_path.is_file():
                    raise MatrixError(f"probe produced no fresh preprocessed source for {unique_stem}")
                preprocessed_hash = sha256_bytes(preprocessed_path.read_bytes())
                previous_preprocessed_hash = preprocessed_hashes.setdefault(desc["source"], preprocessed_hash)
                if preprocessed_hash != previous_preprocessed_hash:
                    raise MatrixError(f"preprocessed source changed across optimization levels for {desc['source']}")
                try:
                    raw_report = json.loads(report_path.read_text())
                except (OSError, json.JSONDecodeError) as exc:
                    raise MatrixError(f"invalid fresh probe report for {unique_stem}: {exc}") from exc
                normalized = normalize_probe(raw_report, desc, opt)
                if completed.returncode == 0 and not raw_report.get("accepted"):
                    raise MatrixError(f"probe exit/result disagreement for {unique_stem}")
                if completed.returncode == 1 and raw_report.get("accepted"):
                    raise MatrixError(f"probe exit/result disagreement for {unique_stem}")
                outputs.append({
                    "kind": desc["kind"], "source": desc["source"],
                    "source_sha256": desc["source_sha256"],
                    "preprocessed_sha256": preprocessed_hash, "optimization": opt,
                    "symbols": normalized["symbols"], "block": normalized["block"],
                })
                if not keep_artifacts:
                    shutil.rmtree(output_folder)
        if validate_reference(root) != reference_hash:
            raise MatrixError("reference ELF changed during matrix run")
        if sha256_bytes(compiler_path.read_bytes()) != compiler_hash:
            raise MatrixError("agbcc compiler binary changed during matrix run")
        for desc in plan:
            if sha256_bytes((root / desc["source"]).read_bytes()) != desc["source_sha256"]:
                raise MatrixError(f"source changed during matrix run: {desc['source']}")
    aggregates = {}
    for opt in OPTS:
        rows = [row for row in outputs if row["optimization"] == opt]
        symbols = [symbol for row in rows for symbol in row["symbols"]]
        aggregates[opt] = {
            "translation_units": len(rows),
            "symbols": len(symbols),
            "strict_symbol_matches": sum(symbol["status"] == "MATCH" for symbol in symbols),
            "symbol_aligned_differing_bytes": sum(symbol["differing_bytes"] for symbol in symbols),
            "symbol_size_mismatches": sum(symbol["actual_size"] != symbol["expected_size"] for symbol in symbols),
            "whole_block_matches": sum(row["block"]["matches"] is True for row in rows),
            "whole_block_differing_bytes": sum(row["block"]["differing_bytes"] for row in rows),
        }
    return {
        "schema": "optimizer-matrix-v1",
        "reference_elf": str(REFERENCE), "reference_elf_sha256": reference_hash,
        "compiler": {"declared_banner": toolchain.get("compiler_banner"),
                     "binary": str(COMPILER_BINARY), "binary_sha256": compiler_hash},
        "toolchain_revision": toolchain.get("revision"),
        "effective_flags": {
            "cpp": ["-P", "-Iinclude"],
            "agbcc": ["-O{optimization}", "-mthumb-interwork"],
            "assembler": ["-mcpu=arm7tdmi"],
        },
        "optimization_order": list(OPTS),
        "descriptors": plan,
        "results": outputs,
        "aggregate_by_optimization": aggregates,
        "interpretation": "Diagnostic compiler-shape evidence only; matching requires separate source review and integrated verification.",
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--plan", action="store_true", help="print validated mechanically derived descriptors, without compiling")
    parser.add_argument("--output", type=Path, default=Path("build/optimizer-matrix.json"))
    parser.add_argument("--keep-artifacts", action="store_true")
    args = parser.parse_args()
    try:
        if args.plan:
            validate_reference(ROOT)
            payload = {
                "reference_elf": str(REFERENCE), "reference_elf_sha256": REFERENCE_SHA256,
                "optimization_order": list(OPTS), "descriptors": descriptor_plan(ROOT),
            }
        else:
            payload = run_matrix(ROOT, args.keep_artifacts)
        rendered = json.dumps(payload, indent=2, sort_keys=True) + "\n"
        if args.plan:
            sys.stdout.write(rendered)
        else:
            destination = (ROOT / args.output).resolve() if not args.output.is_absolute() else args.output
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_text(rendered)
            print(f"wrote {destination}")
        return 0
    except MatrixError as exc:
        print(f"optimizer matrix: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
