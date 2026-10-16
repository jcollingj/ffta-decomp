#!/usr/bin/env python3
"""Rebuild the eight recorded natural-source trials and optionally reprobe."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
HISTORICAL_BASELINE_SHA256 = "6e3a6cfcb7f7b4d20a85330f5dbff9039d604abc06f0e4cf6163a51a639380f9"
CURRENT_BEST_SHA256 = "79c3174949b6ddd94bbd5574fdced852d4ca84c0c2e04d23434abfa7783ca298"

PARENT_LINKS = """if (parent->child_a == node)
            parent->child_a = 0;
        else if (parent->child_b == node)
            parent->child_b = 0;"""
PARENT_LINKS_BRACED = """if (parent->child_a == node) { parent->child_a = 0; }
        else if (parent->child_b == node) { parent->child_b = 0; }"""
INVERTED_ROOT = """    if (parent != 0) {
        if (parent->child_a == node)
            parent->child_a = 0;
        else if (parent->child_b == node)
            parent->child_b = 0;
        node->parent = 0;
    } else {
        owner->tree_root = 0;
        subtree = node->child_a;
        if (subtree != 0) {
            owner->tree_root = subtree;
            node->child_a = 0;
        } else {
            subtree = node->child_b;
            if (subtree != 0) {
                owner->tree_root = subtree;
                node->child_b = 0;
            }
        }
        if (owner->tree_root != 0)
            owner->tree_root->parent = 0;
    }"""
ROOT_BRANCH = """    if (parent == 0) {
        owner->tree_root = 0;
        subtree = node->child_a;
        if (subtree != 0) {
            owner->tree_root = subtree;
            node->child_a = 0;
        } else {
            subtree = node->child_b;
            if (subtree != 0) {
                owner->tree_root = subtree;
                node->child_b = 0;
            }
        }
        if (owner->tree_root != 0)
            owner->tree_root->parent = 0;
    } else {
        if (parent->child_a == node)
            parent->child_a = 0;
        else if (parent->child_b == node)
            parent->child_b = 0;
        node->parent = 0;
    }"""
LINKS_BLOCK = """            if ((int)subtree->unknown00 < (int)cursor->unknown00) {
                child = cursor->child_a;
                if (child == 0) {
                    cursor->child_a = subtree;
                    break;
                }
            } else {
                child = cursor->child_b;
                if (child == 0) {
                    cursor->child_b = subtree;
                    break;
                }
            }"""
LINKS_UNINITIALIZED_EDGE = """            {
                struct TreeCountNode **edge;
                edge = (int)subtree->unknown00 < (int)cursor->unknown00 ?
                       &cursor->child_a : &cursor->child_b;
                child = *edge;
                if (child == 0) { *edge = subtree; break; }
            }"""
LINKS_INITIALIZED_EDGE = """            {
                struct TreeCountNode **edge =
                    (int)subtree->unknown00 < (int)cursor->unknown00 ?
                    &cursor->child_a : &cursor->child_b;
                child = *edge;
                if (child == 0) { *edge = subtree; break; }
            }"""

TRIALS = [
    ("v01_braces", [(PARENT_LINKS, PARENT_LINKS_BRACED)],
     "bf23dbd4f6981e6a13f57415585f979000cfe62e7665e0576c8bbd9ccc81f722", 145, 174, 145),
    ("v02_inverted_root_test", [(ROOT_BRANCH, INVERTED_ROOT)],
     "79c3174949b6ddd94bbd5574fdced852d4ca84c0c2e04d23434abfa7783ca298", 137, 174, 137),
    ("v03_while_loop", [("for (;;) {\n            if ((int)subtree->unknown00", "while (1) {\n            if ((int)subtree->unknown00")],
     "81d1e7c32186d1935759067325e6ccd84fa188b0eddb10cf0f56f6555d50baaa", 145, 174, 145),
    ("v04_signed_spelling", [("(int)subtree->unknown00 < (int)cursor->unknown00",
                               "((signed int)subtree->unknown00) < ((signed int)cursor->unknown00)")],
     "46b7abf5954429dc181798dd43ea44cf5bf780be84c3963d1c00b13ccb8e1f4e", 145, 174, 145),
    ("v05_link_pointer", [(LINKS_BLOCK, LINKS_UNINITIALIZED_EDGE)],
     "ea602b030cf183779c177ee242132b013de0d24cf58ea4cae00a6ea611167dac", 163, 168, 161),
    ("v06_move_parent_clear", [("        node->parent = 0;\n    }\n\n    subtree = node->child_a;",
                                "    }\n\n    subtree = node->child_a;")],
     "c9e841289c06bfc537a8d2a112a09551cb322edc62985b7dbb3dee97d493de75", 143, 170, 141),
    ("v07_child_local_assignment", [(LINKS_BLOCK, LINKS_INITIALIZED_EDGE)],
     "570473247f5d31dc963025a5cf3d10287320d6425765027790b531586aaca5fb", 163, 168, 161),
    ("v08_pointer_compare_reordered", [(PARENT_LINKS,
       "if (node == parent->child_a)\n            parent->child_a = 0;\n        else if (node == parent->child_b)\n            parent->child_b = 0;")],
     "159ce89e6996c0bec0118c20b61b14d852ac1c4088ebefb4536029f082004326", 145, 174, 145),
]


def build_sources(current_source):
    current_hash = hashlib.sha256(current_source.encode()).hexdigest()
    if current_hash == CURRENT_BEST_SHA256:
        # Recover the exact first candidate from the chosen equivalent v02 form.
        baseline = current_source.replace(INVERTED_ROOT, ROOT_BRANCH, 1)
    elif current_hash == HISTORICAL_BASELINE_SHA256:
        baseline = current_source
    else:
        raise ValueError("tracked candidate matches neither recorded source hash")
    baseline_hash = hashlib.sha256(baseline.encode()).hexdigest()
    if baseline_hash != HISTORICAL_BASELINE_SHA256:
        raise ValueError(f"historical baseline reconstruction hash mismatch: {baseline_hash}")
    results = []
    for name, edits, expected_hash, expected_diff, expected_size, expected_function_diff in TRIALS:
        source = baseline
        for before, after in edits:
            if before not in source:
                raise ValueError(f"{name}: source edit anchor not found")
            source = source.replace(before, after, 1)
        actual_hash = hashlib.sha256(source.encode()).hexdigest()
        if actual_hash != expected_hash:
            raise ValueError(f"{name}: reconstructed SHA-256 {actual_hash}, expected {expected_hash}")
        results.append((name, source, expected_diff, expected_size, expected_function_diff))
    return results


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--probe", action="store_true", help="rerun all eight bounded target probes")
    args = parser.parse_args()
    baseline_path = ROOT / "src/nonmatching/tree_detach.c"
    trials = build_sources(baseline_path.read_text())
    print("trial source hashes verified")
    if not args.probe:
        for name, _, expected_diff, expected_size, expected_function_diff in trials:
            print(f"{name}: function diff {expected_function_diff}, block diff {expected_diff}, symbol size {expected_size}")
        return 0
    for name, source, expected_diff, expected_size, expected_function_diff in trials:
        source_hash = hashlib.sha256(source.encode()).hexdigest()
        stem = f"{name}_{source_hash[:12]}"
        scratch = ROOT / "build/probes/tree_detach_trials" / f"{stem}.c"
        scratch.parent.mkdir(parents=True, exist_ok=True)
        scratch.write_text(source)
        folder = ROOT / "build/probes" / stem
        if folder.exists():
            shutil.rmtree(folder)
        result = subprocess.run([sys.executable, "tools/decomp/probe.py", str(scratch),
                                 "--start", "0xC7858", "--end", "0xC7908"], cwd=ROOT,
                                capture_output=True, text=True)
        if result.returncode not in (0, 1):
            raise RuntimeError(f"{name}: probe failed with {result.returncode}: {result.stderr or result.stdout}")
        report_path = folder / "comparison.json"
        elf_path = folder / "candidate.elf"
        binary_path = folder / "candidate.bin"
        if not report_path.is_file() or not elf_path.is_file() or not binary_path.is_file():
            raise RuntimeError(f"{name}: probe did not produce fresh comparison artifacts")
        report = json.loads(report_path.read_text())
        nm = subprocess.check_output(["arm-none-eabi-nm", "-S", str(elf_path)], text=True)
        match = re.search(r"^[0-9a-fA-F]+\s+([0-9a-fA-F]+)\s+T\s+DetachTreeNode$", nm, re.M)
        if not match:
            raise RuntimeError(f"{name}: DetachTreeNode symbol absent")
        symbol_size = int(match.group(1), 16)
        candidate = binary_path.read_bytes()
        original = (ROOT / "baserom.gba").read_bytes()[0xC7858:0xC7908]
        common_size = min(symbol_size, 174)
        common_symbol_diff = sum(a != b for a, b in zip(original[:common_size], candidate[:common_size]))
        function_diff = common_symbol_diff + abs(174 - symbol_size)
        if (report["differing_bytes"] != expected_diff or symbol_size != expected_size or
                function_diff != expected_function_diff):
            raise RuntimeError(f"{name}: function diff={function_diff}, block diff={report['differing_bytes']}, size={symbol_size}")
        print(f"{name}: source hash {source_hash}, function diff {function_diff}, block diff {expected_diff}, symbol size {symbol_size}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
