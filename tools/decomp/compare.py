#!/usr/bin/env python3
"""Incrementally build and compare recovered functions with original ROM bytes."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import time

ROOT = Path(__file__).resolve().parents[2]


def compare_bytes(expected, actual, start):
    changes = [dict(offset=f'0x{start + i:06X}', expected=a, actual=b)
               for i, (a, b) in enumerate(zip(expected, actual)) if a != b]
    return dict(matches=expected == actual, expected_size=len(expected), actual_size=len(actual),
                differing_bytes=len(changes) + abs(len(expected) - len(actual)), first_differences=changes[:16])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('function', nargs='?', help='function name; default all inventoried functions')
    parser.add_argument('--no-build', action='store_true')
    parser.add_argument('--disassemble', action='store_true', help='write side-by-side input assembly files under build/compare')
    args = parser.parse_args()
    begin = time.perf_counter()
    manifest = json.loads((ROOT / 'config/decomp_regions.json').read_text())
    functions = [f for f in manifest['functions'] if not args.function or f['name'] == args.function]
    if not functions:
        parser.error('no matching inventoried function')
    base = (ROOT / 'baserom.gba').read_bytes()
    if hashlib.sha1(base).hexdigest() != manifest['rom_sha1']:
        raise SystemExit('wrong base ROM')
    if not args.no_build:
        subprocess.run(['make', 'build/ffta.gba'], cwd=ROOT, check=True)
    built = (ROOT / 'build/ffta.gba').read_bytes()
    symbols = {}
    for line in subprocess.check_output(['arm-none-eabi-nm', '-S', str(ROOT / 'build/ffta.elf')], text=True).splitlines():
        columns = line.split()
        if len(columns) == 4:
            symbols[columns[3]] = (int(columns[0], 16) & ~1, int(columns[1], 16))
    reports = []
    for function in functions:
        start, end, name = function['start'], function['end'], function['name']
        address, size = symbols.get(name, (-1, -1))
        result = compare_bytes(base[start:end], built[start:end], start)
        result.update(name=name, symbol_address=f'{address:#x}', symbol_size=size,
                      layout_matches=(address == 0x08000000 + start and size == end - start))
        reports.append(result)
        print(f"{name}: {'MATCH' if result['matches'] and result['layout_matches'] else 'DIFF'}; {end-start} bytes at {start:#x}")
        for diff in result['first_differences']:
            print(f"  {diff['offset']}: expected {diff['expected']:02x}, got {diff['actual']:02x}")
        if args.disassemble:
            folder = ROOT / 'build/compare'
            folder.mkdir(exist_ok=True)
            for label, path in [('original', ROOT / 'baserom.gba'), ('built', ROOT / 'build/ffta.gba')]:
                command = ['arm-none-eabi-objdump', '-D', '-b', 'binary', '-m', 'arm',
                           f'--start-address={start}', f'--stop-address={end}']
                if function['mode'] == 'thumb':
                    command += ['-M', 'force-thumb']
                with (folder / f'{name}.{label}.s').open('w') as output:
                    subprocess.run(command + [str(path)], stdout=output, check=True)
    elapsed = time.perf_counter() - begin
    (ROOT / 'build/function-comparison.json').write_text(json.dumps(dict(seconds=elapsed, functions=reports), indent=2)+'\n')
    print(f'Build + comparison: {elapsed:.3f}s')
    if not all(r['matches'] and r['layout_matches'] for r in reports):
        raise SystemExit(1)


if __name__ == '__main__':
    main()
