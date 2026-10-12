#!/usr/bin/env python3
"""Print a verified, bounded research packet for one inventoried function."""

import argparse
from collections import defaultdict
import hashlib
import json
from pathlib import Path
import shlex
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[2]
EXPECTED_ROM_SHA1 = '4ac05441f4de70a4ec3dd932116346c61b8783d9'
EXPECTED_ROM_SIZE = 0x1000000
CAVEATS = [
    'The interval is a reviewed bound, not proof that every byte is function code; it may include a literal pool or padding.',
    'Verify the function boundary and control-flow back-edges against the original instructions before making source claims.',
    'A source-manifest count of one does not prove the translation unit has no unmanifested functions; a probe places the whole source file at the requested start, not a verified function extent.',
]


def _checked_bounds(entry, label, rom_size):
    start, end = entry.get('start'), entry.get('end')
    if (not isinstance(start, int) or isinstance(start, bool) or
            not isinstance(end, int) or isinstance(end, bool) or
            not 0 <= start < end <= rom_size):
        raise ValueError(f'invalid {label} interval for {entry.get("name", "<unnamed>")}')
    if entry.get('mode') == 'thumb' and (start & 1 or end & 1):
        raise ValueError(f'Thumb interval boundaries must be halfword aligned: {entry.get("name")}')
    return start, end


def _references(matching_manifest, candidate_manifest, rom_size):
    """Merge same-name references, rejecting any disagreement in address bounds."""
    references = {}
    for category, manifest in (('matching', matching_manifest), ('candidate', candidate_manifest)):
        for entry in manifest.get('functions', []):
            _checked_bounds(entry, category, rom_size)
            name = entry.get('name')
            if not isinstance(name, str) or not name:
                raise ValueError(f'{category} manifest contains a function without a name')
            previous = references.get(name)
            if previous and (previous['start'], previous['end']) != (entry['start'], entry['end']):
                raise ValueError(f'conflicting reference bounds for {name}')
            # The matching ledger is authoritative when both agree.
            if previous is None or category == 'matching':
                references[name] = dict(entry, _category=category)
    return references


def build_packet(function_name, matching_manifest, candidate_manifest, rom,
                 expected_sha1=EXPECTED_ROM_SHA1, expected_size=EXPECTED_ROM_SIZE):
    """Build a packet from manifest objects and immutable ROM bytes (easy to fixture)."""
    if len(rom) != expected_size:
        raise ValueError(f'wrong base ROM size: expected {expected_size} bytes, got {len(rom)}')
    rom_sha1 = hashlib.sha1(rom).hexdigest()
    if rom_sha1 != expected_sha1:
        raise ValueError(f'wrong base ROM SHA-1: expected {expected_sha1}, got {rom_sha1}')
    references = _references(matching_manifest, candidate_manifest, expected_size)
    entry = references.get(function_name)
    if entry is None:
        raise ValueError(f'unknown function: {function_name}')

    start, end = entry['start'], entry['end']
    source = entry.get('source')
    category = entry['_category']
    source_names = defaultdict(set)
    for referenced_name, reference in references.items():
        if reference.get('source'):
            source_names[reference['source']].add(referenced_name)
    packet = {
        'function': function_name,
        'classification': 'admitted matching function' if category == 'matching' else 'research candidate',
        'source_status': 'known' if source else 'unknown',
        'source': source,
        'mode': entry.get('mode'),
        'start_decimal': start,
        'end_decimal': end,
        'start_hex': f'0x{start:X}',
        'end_hex': f'0x{end:X}',
        'interval_size': end - start,
        'rom_size': len(rom),
        'rom_sha1': rom_sha1,
        'interval_sha256': hashlib.sha256(rom[start:end]).hexdigest(),
        'caveats': list(CAVEATS),
    }
    if category == 'matching':
        packet['comparison_command'] = f'python3 tools/decomp/compare.py {function_name}'
        packet['probe_command'] = None
        packet['command_note'] = 'Use the named-function matching comparison.'
    elif source and len(source_names[source]) == 1:
        command = ['python3', 'tools/decomp/probe.py', source,
                   '--start', f'0x{start:X}', '--end', f'0x{end:X}']
        packet['probe_command'] = shlex.join(command)
        packet['comparison_command'] = None
        packet['command_note'] = 'Only one function in the manifests names this source; unmanifested definitions may still affect placement.'
    else:
        packet['probe_command'] = None
        packet['comparison_command'] = None
        if source:
            packet['command_note'] = ('Multiple manifest functions share this source file; compiling the whole translation unit at this function start would misplace its other functions.')
        else:
            packet['command_note'] = 'No source path is recorded, so no compile or comparison command can be formed.'
    return packet


def load_packet(function_name, root=ROOT):
    root = Path(root)
    matching = json.loads((root / 'config/decomp_regions.json').read_text())
    candidates = json.loads((root / 'config/decomp_candidates.json').read_text())
    if matching.get('rom_sha1') != EXPECTED_ROM_SHA1:
        raise ValueError('matching manifest does not declare the expected USA ROM SHA-1')
    if matching.get('rom_size') != EXPECTED_ROM_SIZE:
        raise ValueError('matching manifest does not declare the expected ROM size')
    rom = (root / 'baserom.gba').read_bytes()
    return build_packet(function_name, matching, candidates, rom,
                        expected_sha1=EXPECTED_ROM_SHA1,
                        expected_size=EXPECTED_ROM_SIZE)


def disassemble_original(packet, root=ROOT):
    if packet.get('mode') != 'thumb':
        raise ValueError(f"unsupported or unknown disassembly mode: {packet.get('mode')}")
    rom_path = Path(root) / 'baserom.gba'
    command = ['arm-none-eabi-objdump', '-D', '-b', 'binary', '-m', 'arm',
               f"--start-address={packet['start_decimal']}",
               f"--stop-address={packet['end_decimal']}", '-M', 'force-thumb', str(rom_path)]
    try:
        subprocess.run(command, check=True, stdout=sys.stdout)
    except FileNotFoundError as exc:
        raise RuntimeError('arm-none-eabi-objdump is required for --disassemble') from exc
    except subprocess.CalledProcessError as exc:
        raise RuntimeError(f'objdump failed with exit status {exc.returncode}') from exc


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('function', help='function name from a matching or candidate manifest')
    output = parser.add_mutually_exclusive_group()
    output.add_argument('--json', action='store_true', help='print the packet as JSON')
    output.add_argument('--disassemble', action='store_true', help='disassemble the original ROM interval to stdout')
    args = parser.parse_args(argv)
    try:
        packet = load_packet(args.function)
        if args.json:
            print(json.dumps(packet, indent=2))
        else:
            print(f"{packet['function']}: {packet['classification']}")
            print(f"  source: {packet['source'] or 'unknown'} ({packet['source_status']})")
            print(f"  bounds: {packet['start_decimal']}..{packet['end_decimal']} "
                  f"({packet['start_hex']}..{packet['end_hex']}, {packet['interval_size']} bytes)")
            print(f"  interval SHA-256: {packet['interval_sha256']}")
            command = packet['comparison_command'] or packet['probe_command'] or 'unavailable'
            print(f'  suggested command: {command}')
            print(f"  command note: {packet['command_note']}")
            for caveat in packet['caveats']:
                print(f'  caveat: {caveat}')
        if args.disassemble:
            disassemble_original(packet)
    except (OSError, ValueError, RuntimeError, json.JSONDecodeError) as exc:
        parser.error(str(exc))


if __name__ == '__main__':
    main()
