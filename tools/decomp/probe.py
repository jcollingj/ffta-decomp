#!/usr/bin/env python3
"""Compile a C candidate at its original address without changing the game build.

Unresolved references inherit addresses and ARM/Thumb types from build/ffta.elf.
Use this for nonmatching candidates whose size would trip the main layout guards.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import time
from compare import compare_bytes
from probe_report import compare_functions, merge_references, text_symbols

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=Path)
    parser.add_argument('--start', required=True, type=lambda s: int(s, 0), help='ROM file offset')
    parser.add_argument('--end', required=True, type=lambda s: int(s, 0), help='exclusive ROM file offset')
    parser.add_argument('--opt', default='2', choices=['0', '1', '2', 's'])
    parser.add_argument('--disassemble', action='store_true', help='save original/candidate disassembly for each referenced function')
    args = parser.parse_args()
    if not 0 <= args.start < args.end <= 0x1000000:
        parser.error('invalid ROM interval')
    start = time.perf_counter()
    folder = ROOT / 'build/probes' / args.source.stem
    folder.mkdir(parents=True, exist_ok=True)
    def run(command):
        subprocess.run([str(part) for part in command], cwd=ROOT, check=True)
    run(['arm-none-eabi-cpp', '-P', '-Iinclude', args.source.resolve(), '-o', folder/'candidate.i'])
    run([ROOT/'tools/.deps/agbcc/gcc/agbcc', '-O'+args.opt, '-mthumb-interwork', folder/'candidate.i', '-o', folder/'candidate.s'])
    run(['arm-none-eabi-as', '-mcpu=arm7tdmi', '-o', folder/'candidate.o', folder/'candidate.s'])
    undefined = [line.split()[-1] for line in subprocess.check_output(
        ['arm-none-eabi-nm', '-u', str(folder/'candidate.o')], text=True).splitlines()]
    symbols = {}
    for line in subprocess.check_output(['arm-none-eabi-readelf', '-sW', str(ROOT/'build/ffta.elf')], text=True).splitlines():
        parts = line.split()
        if len(parts) >= 8 and parts[0].endswith(':') and parts[0][:-1].isdigit() and parts[6] != 'UND':
            symbols[parts[7]] = (int(parts[1], 16), parts[3])
    refs = ['.thumb']
    for name in undefined:
        if name not in symbols:
            raise SystemExit(f'Unknown external symbol {name}; document its address/type first')
        value, kind = symbols[name]
        refs.append(f'.global {name}')
        if kind == 'FUNC':
            refs.append(f'.type {name}, %function')
        refs.append(f'{".thumb_set" if kind == "FUNC" and value & 1 else ".set"} {name}, {value:#x}')
    (folder/'refs.s').write_text('\n'.join(refs)+'\n')
    run(['arm-none-eabi-as', '-mcpu=arm7tdmi', '-o', folder/'refs.o', folder/'refs.s'])
    (folder/'probe.ld').write_text(f'SECTIONS {{ . = {0x08000000 + args.start:#x}; .text : {{ *(.text) *(.rodata*) }} /DISCARD/ : {{ *(*) }} }}\n')
    run(['arm-none-eabi-ld', '-T', folder/'probe.ld', '-o', folder/'candidate.elf', folder/'candidate.o', folder/'refs.o'])
    run(['arm-none-eabi-objcopy', '-O', 'binary', folder/'candidate.elf', folder/'candidate.bin'])
    base = (ROOT/'baserom.gba').read_bytes()
    if hashlib.sha1(base).hexdigest() != '4ac05441f4de70a4ec3dd932116346c61b8783d9':
        raise SystemExit('wrong base ROM')
    expected = base[args.start:args.end]
    actual = (folder/'candidate.bin').read_bytes()
    result = compare_bytes(expected, actual, args.start)
    manifest = json.loads((ROOT/'config/decomp_regions.json').read_text())
    research = json.loads((ROOT/'config/decomp_candidates.json').read_text())
    references = merge_references(manifest['functions'], research['functions'])
    candidate_symbols = text_symbols(subprocess.check_output(
        ['arm-none-eabi-nm', '-S', str(folder/'candidate.elf')], text=True))
    reports = compare_functions(base, actual, args.start, references, candidate_symbols)
    result.update(functions=reports, optimization=args.opt, source=str(args.source),
                  seconds=time.perf_counter()-start)
    for report in reports:
        name = report['name']
        if report['status'] == 'UNINVENTORIED':
            print(f'{name}: no reviewed reference bounds')
            continue
        print(f"{name}: {report['status']}; size {report['actual_size']}/{report['expected_size']}; "
              f"address delta {report['address_delta']:+#x}; {report['differing_bytes']} differing bytes (symbol-aligned)")
        if args.disassemble:
            for label, path, address in [('original', ROOT/'baserom.gba', report['reference_start']),
                                         ('candidate', folder/'candidate.bin', report['actual_start'])]:
                length = report['expected_size'] if label == 'original' else report['actual_size']
                command = ['arm-none-eabi-objdump', '-D', '-b', 'binary', '-m', 'arm',
                           f'--start-address={address}', f'--stop-address={address+length}']
                if label == 'candidate':
                    command += [f'--adjust-vma={args.start}']
                if references[name]['mode'] == 'thumb':
                    command += ['-M', 'force-thumb']
                with (folder/f'{name}.{label}.s').open('w') as output:
                    subprocess.run(command+[str(path)], stdout=output, check=True)
    layouts_match = all(r['status'] != 'DIFF' for r in reports)
    result['accepted'] = result['matches'] and layouts_match
    (folder/'comparison.json').write_text(json.dumps(result, indent=2)+'\n')
    print(f"{'MATCH' if result['accepted'] else 'DIFF'}: {len(actual)} candidate / {len(expected)} original bytes; {time.perf_counter()-start:.3f}s")
    for diff in result['first_differences']:
        print(f"  {diff['offset']}: expected {diff['expected']:02x}, got {diff['actual']:02x}")
    print(f'Assembly and ELF: {folder}')
    raise SystemExit(0 if result['accepted'] else 1)


if __name__ == '__main__':
    main()
