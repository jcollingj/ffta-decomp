#!/usr/bin/env python3
"""Fix the GBA header complement checksum (byte 0xBD) in place."""
import sys

path = sys.argv[1]
with open(path, "r+b") as f:
    data = bytearray(f.read(0xC0))
    chk = (-(0x19 + sum(data[0xA0:0xBD]))) & 0xFF
    if data[0xBD] != chk:
        f.seek(0xBD)
        f.write(bytes([chk]))
        print(f"gbafix: header checksum set to 0x{chk:02X}")
