#!/usr/bin/env python3
"""Audit the declared ROM coverage against the matching build and incbin sources.

Categories describe implementation provenance, not semantic understanding.
The reviewed region/function manifest is authoritative; this tool does not
pretend to discover functions or recognize decompiled C automatically.
"""
import argparse
from collections import Counter
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
KINDS = {'copied', 'source_data', 'generated', 'recovered_asm', 'decompiled_c'}
INCBIN = re.compile(r'^\s*\.incbin\s+"baserom\.gba",\s*(0x[0-9a-fA-F]+|\d+),\s*(0x[0-9a-fA-F]+|\d+)\s*(?:@.*)?$', re.M)


def measure(manifest):
    cursor = 0
    totals = Counter({kind: 0 for kind in KINDS})
    for region in manifest['regions']:
        start, end = region['start'], region['end']
        if start != cursor or end <= start or end > manifest['rom_size']:
            raise ValueError(f'gap, overlap, or invalid bounds at {start:#x}')
        if region['kind'] not in KINDS:
            raise ValueError(f'unknown category: {region["kind"]}')
        totals[region['kind']] += end - start
        cursor = end
    if cursor != manifest['rom_size']:
        raise ValueError('region map does not cover the entire ROM')
    functions = manifest['functions']
    addresses = set()
    recovered = 0
    for function in functions:
        start, end = function['start'], function['end']
        if start in addresses or not 0 <= start < end <= cursor:
            raise ValueError('duplicate or invalid function')
        addresses.add(start)
        if function['status'] == 'matching_c':
            if not any(r['kind'] == 'decompiled_c' and r['start'] <= start < end <= r['end']
                       for r in manifest['regions']):
                raise ValueError('matching function must belong to a decompiled C region')
            recovered += 1
    return {
        'rom_bytes': cursor, 'bytes_by_kind': dict(sorted(totals.items())),
        'source_or_generated_bytes': cursor - totals['copied'],
        'source_or_generated_percent': (cursor - totals['copied']) * 100 / cursor,
        'matching_c_functions': recovered,
        'inventoried_functions': len(functions),
        'total_functions': len(functions) if manifest['code_inventory_complete'] else None,
        'code_completion_percent': (100 * recovered / len(functions))
            if manifest['code_inventory_complete'] and functions else None,
        'code_inventory_complete': manifest['code_inventory_complete'],
    }


def parse_build_objects(makefile):
    """Return explicit OBJS tokens in order; reject unsupported make syntax."""
    lines = makefile.splitlines()
    assignments = [(index, line) for index, line in enumerate(lines)
                   if re.match(r'^\s*OBJS\b', line)]
    if len(assignments) != 1:
        raise ValueError('expected exactly one explicit OBJS assignment')

    index, line = assignments[0]
    if not re.match(r'^\s*OBJS\s*:=', line):
        raise ValueError('OBJS must use an explicit := assignment')
    body = re.sub(r'^\s*OBJS\s*:=', '', line, count=1)
    pieces = []
    while True:
        stripped = body.rstrip()
        continued = stripped.endswith('\\')
        pieces.append(stripped[:-1] if continued else body)
        if not continued:
            break
        index += 1
        if index >= len(lines):
            raise ValueError('unterminated OBJS continuation')
        body = lines[index].lstrip()

    objects = ' '.join(pieces).split()
    if not objects:
        raise ValueError('OBJS assignment is empty')
    for obj in objects:
        if (not re.fullmatch(r'build/[A-Za-z0-9_.+-]+\.o', obj)
                or Path(obj).parent != Path('build') or Path(obj).name in ('.o', '..o')):
            raise ValueError(f'unsupported OBJS token: {obj}')
    if len(set(objects)) != len(objects):
        raise ValueError('duplicate object in OBJS assignment')
    return objects


def audit(manifest, root, rom):
    report = measure(manifest)
    data = rom.read_bytes()
    sha1 = hashlib.sha1(data).hexdigest()
    if len(data) != manifest['rom_size'] or sha1 != manifest['rom_sha1']:
        raise ValueError('progress requires the exact matching original build')
    # Check every source in the current matching build, including sources not
    # listed by the manifest, so a newly introduced incbin cannot go unnoticed.
    makefile = (root / 'Makefile').read_text()
    sources = set()
    for obj in parse_build_objects(makefile):
        stem = Path(obj).stem
        candidates = [root / folder / f'{stem}{suffix}'
                      for folder, suffix in [('asm', '.s'), ('data', '.s'), ('src', '.c')]]
        found = [p for p in candidates if p.exists()]
        if len(found) != 1:
            raise ValueError(f'cannot resolve a unique source for {obj}')
        sources.add(found[0].relative_to(root).as_posix())
    expected, actual = [], []
    for region in manifest['regions']:
        if not (root / region['source']).is_file():
            raise ValueError(f'missing region source: {region["source"]}')
        if region['kind'] == 'copied':
            expected.append((region['source'], region['start'], region['end']))
        elif region['kind'] != 'generated' and region['source'] not in sources:
            raise ValueError('source-backed region is not present in the matching build')
    for source in sources:
        text = (root / source).read_text()
        matches = list(INCBIN.finditer(text))
        if len(matches) != len(re.findall(r'^\s*\.incbin\b', text, re.M)):
            raise ValueError('unrecognized incbin; update the auditor before reporting coverage')
        for match in matches:
            start, size = (int(v, 0) for v in match.groups())
            actual.append((source, start, start + size))
    if sorted(expected) != sorted(actual):
        raise ValueError('copied region manifest differs from build source incbin directives')
    report.update(verified_utc=datetime.now(timezone.utc).isoformat(), rom_sha1=sha1,
                  matching_build_verified=True,
                  method='Reviewed region manifest; contiguous coverage, matching ROM hash, and build-source incbin audit. Function inventory incomplete.')
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / 'docs/decomp-progress.json')
    args = parser.parse_args()
    manifest = json.loads((ROOT / 'config/decomp_regions.json').read_text())
    report = audit(manifest, ROOT, ROOT / 'build/ffta.gba')
    args.output.write_text(json.dumps(report, indent=2) + '\n')
    print(f"Matching C functions: {report['matching_c_functions']}; total game functions: unknown")
    print(f"Source/generated: {report['source_or_generated_bytes']:,} / {report['rom_bytes']:,} ROM bytes ({report['source_or_generated_percent']:.6f}%)")
    print(f"Copied from base ROM: {report['bytes_by_kind']['copied']:,} bytes")
    print('This is ROM provenance coverage, not a percentage of engine decompilation.')
    print(args.output)


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError) as error:
        raise SystemExit(str(error))
