#!/usr/bin/env python3
"""Cached ROM references for investigation, not automatic code classification.

Thumb BL encodings are scanned at halfword alignment; ROM pointers at word
alignment. Data can resemble instructions/pointers. Confirm each hit against
control flow before using it to classify a function or claim recovered code.
"""
import argparse
import hashlib
from pathlib import Path
import sqlite3
import struct
import time

ROOT = Path(__file__).resolve().parents[2]
SHA1 = '4ac05441f4de70a4ec3dd932116346c61b8783d9'
VERSION = '1'
BASE = 0x08000000


def thumb_bl_target(offset, first, second):
    if first & 0xF800 != 0xF000 or second & 0xF800 != 0xF800:
        return None
    displacement = ((first & 0x7FF) << 12) | ((second & 0x7FF) << 1)
    if displacement & 0x400000:
        displacement -= 0x800000
    return BASE + offset + 4 + displacement


def ensure_index(connection, data, force=False):
    connection.execute('CREATE TABLE IF NOT EXISTS metadata (key TEXT PRIMARY KEY, value TEXT)')
    metadata = dict(connection.execute('SELECT key, value FROM metadata'))
    if not force and metadata == {'sha1': SHA1, 'version': VERSION}:
        return False
    with connection:
        connection.execute('DROP TABLE IF EXISTS refs')
        connection.execute('CREATE TABLE refs (target INTEGER, source INTEGER, kind TEXT)')
        rows = []
        halfwords = memoryview(data).cast('H')
        # Host-independent decoding (macOS/Linux targets are normally little endian).
        import sys
        if sys.byteorder != 'little':
            halfwords = [item[0] for item in struct.iter_unpack('<H', data)]
        for i in range(len(halfwords) - 1):
            target = thumb_bl_target(i * 2, halfwords[i], halfwords[i + 1])
            if target is not None and BASE <= target < BASE + len(data):
                rows.append((target, BASE + i * 2, 'thumb_bl_candidate'))
        for i, (value,) in enumerate(struct.iter_unpack('<I', data)):
            if BASE <= value < BASE + len(data):
                rows.append((value & ~1, BASE + i * 4, 'rom_pointer_candidate'))
        connection.executemany('INSERT INTO refs VALUES (?, ?, ?)', rows)
        connection.execute('CREATE INDEX refs_target ON refs(target)')
        connection.execute('DELETE FROM metadata')
        connection.executemany('INSERT INTO metadata VALUES (?, ?)', [('sha1', SHA1), ('version', VERSION)])
    return True


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('target', type=lambda value: int(value, 0), help='ROM address or file offset')
    parser.add_argument('--rebuild', action='store_true')
    parser.add_argument('--limit', type=int, default=40)
    args = parser.parse_args()
    start = time.perf_counter()
    data = (ROOT / 'baserom.gba').read_bytes()
    if hashlib.sha1(data).hexdigest() != SHA1:
        raise SystemExit('wrong base ROM')
    target = (args.target + BASE if args.target < BASE else args.target) & ~1
    path = ROOT / 'build/research/xrefs.sqlite'
    path.parent.mkdir(parents=True, exist_ok=True)
    with sqlite3.connect(path) as connection:
        rebuilt = ensure_index(connection, data, args.rebuild)
        count = connection.execute('SELECT COUNT(*) FROM refs WHERE target = ?', (target,)).fetchone()[0]
        rows = connection.execute('SELECT source, kind FROM refs WHERE target = ? ORDER BY source LIMIT ?',
                                  (target, max(1, args.limit))).fetchall()
    for source, kind in rows:
        print(f'{source:#010x} -> {target:#010x}  {kind}')
    print(f'{count} candidates; {"rebuilt" if rebuilt else "cached"} index; {time.perf_counter()-start:.3f}s')
    print('Confirm code/data and caller boundaries before treating candidates as proven references.')


if __name__ == '__main__':
    main()
