# AllocatorRecenter recovery notes

## Original evidence and semantics

The USA ROM SHA-1 is `4ac05441f4de70a4ec3dd932116346c61b8783d9`. Decode the original interval with:

```sh
python3 - <<'PY'
from pathlib import Path
Path('/tmp/allocator-recenter-original.bin').write_bytes(
    Path('baserom.gba').read_bytes()[0x6D38:0x6DA0])
PY
arm-none-eabi-objdump -D -b binary -m arm -M force-thumb \
  --adjust-vma=0x08006D38 /tmp/allocator-recenter-original.bin
```

`AllocatorRecenter` begins at ROM offset `0x6D38` and ends exclusively at `0x6D9E`: 102 symbol bytes. The two bytes through `0x6DA0` are zero alignment. Key control flow:

- `0x6D38–0x6D50` reads the little-endian u16 head offset at owner `+0`, forms `base + offset*4` when nonzero, and leaves a null pointer otherwise.
- `0x6D52–0x6D5A` handles a null head: the conditional branch at `0x6D54` skips the count path, stores zero at owner `+2/+3` at `0x6D56/0x6D58`, then jumps to the function exit at `0x6D5A` (`0x6D98`). For a nonnull head, `0x6D5C–0x6D64` reads count at owner `+4` and shifts it right one.
- `0x6D66` is an unconditional branch to `0x6D7E`, the decrement/check block. It skips the link load on loop entry.
- `0x6D68–0x6D7C` reads the next little-endian u16 offset from node `+10`, converts nonzero offsets to `base + offset*4`, and holds null for a zero offset.
- `0x6D7E–0x6D82` decrements the shifted count and branches back to the link load only when the result is signed-greater-than-zero.
- `0x6D84–0x6D96` converts a null result to zero or narrows `(node - base) / 4` to u16, then writes that value little-endian at owner `+2`.

Therefore the number of followed links is `max((count >> 1) - 1, 0)`. Counts 0, 1, 2, and 3 retain the head. Count 4 follows one link. If a null successor is reached and another positive loop iteration remains, the original code dereferences null; the C candidate deliberately adds no malformed-chain guard.

The first candidate revision incorrectly translated the loop as an unconditional first hop. Its do/while fixture repeated that same mistake, so its behavioral result is obsolete. Parent review identified the missed initial branch to `0x6D7E`; the current source and independent expected cases account for it. This task prompt should have named the initial branch target in addition to the back-edge, despite existing lesson L007.

## Current candidate and verification

The source is `src/nonmatching/allocator_recenter.c`. It keeps all pointer arithmetic within a single owner/node allocation, uses signed `int` for the decremented count, and narrows the final word offset to u16. The harness includes one-node fixtures whose next field contains a valid poison offset for counts 0/1/2/3; these prove the link is not read. It also checks count 4 following exactly one link, a null successor at count 4, longer lists, owner/node byte decoys, and whole-buffer equality with only owner bytes `+2/+3` changed.

Focused test:

```sh
python3 -m unittest tests/test_allocator_recenter_c.py -v
```

Result: pass. The bounded target probe is:

```sh
python3 tools/decomp/probe.py src/nonmatching/allocator_recenter.c \
  --start 0x6d38 --end 0x6da0
```

Current candidate is an honest `DIFF`: 108 candidate bytes versus the 104-byte interval, 70 differing bytes. `AllocatorRecenter` is 106 bytes (`nm -S` reports `08006d38 0000006a`), compared with the original 102-byte symbol. It is not matching recovery credit.

## Bounded natural-source probes

The following semantically equivalent variants were compiled at `-O2` with the pinned agbcc through `tools/decomp/probe.py`. “Block” is candidate bytes emitted for the interval; “diff” is differing bytes against the original interval. The exact base and transformations are included below; hashes identify the resulting source text.

| Shape | Source SHA-1 | Symbol bytes | Block bytes | Diff bytes |
|---|---|---:|---:|---:|
| Direct `while (--remaining > 0)` with offset head check | `5fab21ee5f0b28db3ac7f258993ed68580dfb8cf` | 96 | 96 | 87 |
| `while (remaining > 1)` and decrement in body | `05c7b89dd65fb6a311831eb835698d3b7c0f5c0c` | 98 | 100 | 84 |
| `for (; remaining > 1; remaining--)` | `7ccaa4f67f68ee9d90a5e8b829c3536af5b197e7` | 98 | 100 | 84 |
| Guarded do/while with `remaining > 1` | `10b8424f8859f86b2dcf9bd8fff9722ca4d016f4` | 98 | 100 | 84 |
| Unsigned pointer-difference shift | `5cd68fdf0791044826996fe158acdfd384ade303` | 90 | 92 | 87 |
| Explicit null-link if/else | `38cde34352ced2d04aef7cc0c83dde23c175659a` | 96 | 96 | 88 |
| Pointer ternary for head, then pointer-null branch | `c02c28526849e8ee970f3b06f34db61e1ca52dfb` | 104 | 104 | 75 |
| Pointer ternary plus `while (remaining > 1)` precheck | `ad7bce7ca64d39f129ca405deadbe3c241f42098` | 106 | 108 | 70 |
| Separate final halfword store | `188c11d2e597e9b645224d29e6c1d9e4a6c0a202` | 92 | 92 | 87 |
| Explicit u16 conversion on count shift | `dab6de3238927a2a0e1488750c3a3108d474af1b` | 96 | 96 | 87 |
| Local declaration order change | `27d764caf936bda032f95423535b81c4c3dedea6` | 96 | 96 | 87 |

The retained source uses the pointer-ternary/head-null branch and signed `while (remaining > 1)` precheck. It is the closest semantically correct trial by byte-difference count, with a 106-byte function and 108-byte block. All trial assemblies and JSON reports remain under ignored `build/probes/`.

This snippet reconstructs the canonical source and the listed source transformations from the retained file. Run it from the repository root; it prints each trial name and SHA-1. The probe measurements in the table were taken from those exact source texts.

```python
from pathlib import Path
import hashlib

retained = Path("src/nonmatching/allocator_recenter.c").read_text()
base = retained.replace(
    "    node = offset ? base + ((u32)offset << 2) : 0;\n"
    "    if (node == 0) {\n"
    "        base[2] = 0;\n"
    "        base[3] = 0;\n"
    "    } else {",
    "    if (offset == 0) {\n"
    "        base[2] = 0;\n"
    "        base[3] = 0;\n"
    "    } else {\n"
    "        node = base + ((u32)offset << 2);",
)
base = base.replace(
    "        while (remaining > 1) {\n"
    "            offset = (u16)((u16)node[10] | ((u16)node[11] << 8));\n"
    "            node = offset ? base + ((u32)offset << 2) : 0;\n"
    "            remaining--;\n"
    "        }",
    "        while (--remaining > 0) {\n"
    "            offset = (u16)((u16)node[10] | ((u16)node[11] << 8));\n"
    "            node = offset ? base + ((u32)offset << 2) : 0;\n"
    "        }",
)
assert hashlib.sha1(base.encode()).hexdigest() == "5fab21ee5f0b28db3ac7f258993ed68580dfb8cf"

link = "            node = offset ? base + ((u32)offset << 2) : 0;"
variants = {
    "direct while": base,
    "while precheck": base.replace(
        "while (--remaining > 0) {", "while (remaining > 1) {"
    ).replace(link, link + "\n            remaining--;"),
    "for decrement": base.replace(
        "while (--remaining > 0) {", "for (; remaining > 1; remaining--) {"
    ),
    "guarded do while": base.replace(
        "        while (--remaining > 0) {\n"
        "            offset = (u16)((u16)node[10] | ((u16)node[11] << 8));\n"
        "            node = offset ? base + ((u32)offset << 2) : 0;\n"
        "        }",
        "        if (remaining > 1) {\n"
        "            do {\n"
        "                offset = (u16)((u16)node[10] | ((u16)node[11] << 8));\n"
        "                node = offset ? base + ((u32)offset << 2) : 0;\n"
        "                remaining--;\n"
        "            } while (remaining > 1);\n"
        "        }",
    ),
    "unsigned difference shift": base.replace(
        "offset = (u16)(((u8 *)node - base) / 4);",
        "offset = (u16)(((u32)((u8 *)node - base)) >> 2);",
    ),
    "explicit null link": base.replace(
        link, "            if (offset == 0)\n                node = 0;\n            else\n                node = base + ((u32)offset << 2);"
    ),
    "pointer head ternary": base.replace(
        "    if (offset == 0) {\n"
        "        base[2] = 0;\n"
        "        base[3] = 0;\n"
        "    } else {\n"
        "        node = base + ((u32)offset << 2);",
        "    node = offset ? base + ((u32)offset << 2) : 0;\n"
        "    if (node == 0) {\n"
        "        base[2] = 0;\n"
        "        base[3] = 0;\n"
        "    } else {",
    ),
    "separate halfword store": base.replace(
        "        base[2] = (u8)offset;\n"
        "        base[3] = (u8)(offset >> 8);",
        "        *(u16 *)(base + 2) = offset;",
    ),
    "explicit u16 shift": base.replace(
        "remaining = (int)(offset >> 1);",
        "remaining = (int)((u16)(offset >> 1));",
    ),
    "declaration reorder": base.replace(
        "    u8 *base = (u8 *)owner;\n"
        "    u8 *node;\n"
        "    u16 offset;\n"
        "    int remaining;",
        "    int remaining;\n"
        "    u16 offset;\n"
        "    u8 *node;\n"
        "    u8 *base = (u8 *)owner;",
    ),
}
for name, source in variants.items():
    print(name, hashlib.sha1(source.encode()).hexdigest())
```
