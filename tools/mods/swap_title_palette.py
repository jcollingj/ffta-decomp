#!/usr/bin/env python3
"""Demo mod: swap red/blue channels of every title screen palette color.

Usage: python3 tools/mods/swap_title_palette.py && make COMPARE=0
Regenerate the matching palette by re-running the carve (see README).
"""
import re

PATH = "data/title_palette.s"

def swap(m):
    v = int(m.group(0), 16)
    r, g, b = v & 31, (v >> 5) & 31, (v >> 10) & 31
    return f"0x{(r << 10) | (g << 5) | b:04X}"

src = open(PATH).read()
open(PATH, "w").write(re.sub(r"0x[0-9A-F]{4}", swap, src))
print("swapped R/B channels in", PATH)
