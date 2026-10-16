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

Current candidate is an honest `DIFF`: 100 emitted candidate bytes versus the 104-byte original interval, with 58 differing interval bytes and 56 symbol-aligned differences. `AllocatorRecenter` is 98 bytes (`nm -S` reports `08006d38 00000062`), compared with the original 102-byte symbol. It is not matching recovery credit.

## Historical natural-source probes (unpacked byte-pointer candidate)

These trials and the following reconstruction belong to the earlier unpacked byte-pointer candidate; they are retained as historical evidence and do not describe the current packed candidate. The following semantically equivalent variants were compiled at `-O2` with the pinned agbcc through `tools/decomp/probe.py`. “Block” is candidate bytes emitted for the interval; “diff” is differing bytes against the original interval. The exact base and transformations are included below; hashes identify the resulting source text.

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

That historical retained source used the pointer-ternary/head-null branch and signed `while (remaining > 1)` precheck. At the time it was the closest semantically correct unpacked trial, with a 106-byte function and 108-byte block. It was superseded by the genuine packed trial below. Trial assemblies and JSON reports remain under ignored `build/probes/`.

This snippet reconstructs the historical canonical source and the listed source transformations from a literal source snapshot, independent of the current candidate file. Run it from the repository root; it prints each trial name and SHA-1. The probe measurements in the table were taken from those exact source texts.

```python
from pathlib import Path
import hashlib

retained = """#include "allocator_recenter.h"

#include "types.h"

void AllocatorRecenter(void *owner)
{
    u8 *base = (u8 *)owner;
    u8 *node;
    u16 offset;
    int remaining;

    offset = (u16)((u16)base[0] | ((u16)base[1] << 8));
    node = offset ? base + ((u32)offset << 2) : 0;
    if (node == 0) {
        base[2] = 0;
        base[3] = 0;
    } else {
        offset = (u16)((u16)base[4] | ((u16)base[5] << 8));
        remaining = (int)(offset >> 1);
        while (remaining > 1) {
            offset = (u16)((u16)node[10] | ((u16)node[11] << 8));
            node = offset ? base + ((u32)offset << 2) : 0;
            remaining--;
        }

        if (node == 0) {
            offset = 0;
        } else {
            offset = (u16)(((u8 *)node - base) / 4);
        }
        base[2] = (u8)offset;
        base[3] = (u8)(offset >> 8);
    }
}

asm(".align 2,0");
"""
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

## Owner/node view trials (2026-10-12)

This subsection records the twelve ordinary-struct trials that were initially mislabeled packed. Their historical best candidate source had SHA-1 `ad7bce7ca64d39f129ca405deadbe3c241f42098`; at that time its symbol occupied `[0x6D38, 0x6DA2)` (106 bytes), and its emitted comparison block occupied `[0x6D38, 0x6DA4)` (108 bytes). The probe reported 70 differing bytes across the full 108-byte candidate/104-byte original interval, and 68 differing bytes within the symbol-sized 106/102 comparison. These are separate historical metrics. That candidate has been superseded by the packed source below. The first twelve owner/node-view follow-ups recorded below were mistakenly called packed. They used ordinary structs without `__attribute__((packed))`; preprocessed output confirmed the missing attribute, the compiler reported owner `sizeof` 8, and generated field accesses used `ldrh`/`strh`. These are ordinary-struct trials, not evidence about packed views. The genuine packed hypothesis and its trials follow in the next section.

The ordinary-struct trial baseline source (source SHA-1 `5e751e2c3994d5898d6a61903df3c91f4b80f7dc`) was:

```c
#include "allocator_recenter.h"
#include "allocator_packed.h"

void AllocatorRecenter(void *owner)
{
    AllocatorOwnerPacked *fields = (AllocatorOwnerPacked *)owner;
    AllocatorNodePacked *node;
    u16 offset;
    int remaining;

    offset = fields->head;
    node = offset ? (AllocatorNodePacked *)((u8 *)owner + (u32)offset * 4) : 0;
    if (node == 0) {
        fields->selection = 0;
    } else {
        remaining = (int)(fields->count >> 1);
        while (remaining > 1) {
            offset = node->next;
            node = offset ? (AllocatorNodePacked *)((u8 *)owner + (u32)offset * 4) : 0;
            remaining--;
        }
        fields->selection = node ? (u16)((u32)((u8 *)node - (u8 *)owner) >> 2) : 0;
    }
}

asm(".align 2,0");
```

The exact one-change-at-a-time transformations and measurements from that ordinary-struct baseline are below. “Interval diff” is the top-level `differing_bytes` value in each probe JSON, comparing the emitted candidate block with the full original interval. “Symbol diff” is the function entry in `functions[]`, comparing candidate and original symbol extents. The interval is 104 original bytes; candidate block size can vary. The changes are expressed literally so each trial can be recreated by applying the stated replacement to the baseline source.

| Trial | Source SHA-1 | Symbol | Block | Interval diff | Symbol diff | Transformation |
|---|---|---:|---:|---:|---:|---|
| 1 | `6fdb712ba1ba7b98c14f32b8af62b1ab25d1d7b0` | 76 | 76 | 91 | 89 | Replace `* 4` in both owner-relative pointer expressions with `<< 2`. |
| 2 | `62af169194b0068629f74f23cc8a1a3b9aaa5b79` | 82 | 84 | 94 | 92 | Replace final `((u32)((u8 *)node - (u8 *)owner) >> 2)` with `(((u8 *)node - (u8 *)owner) / 4)`. |
| 3 | `d5daa4caa2c12f179aa01c23a771cfc91419e6ba` | 74 | 76 | 95 | 93 | Change loop condition to `while (--remaining > 0)` and remove the body decrement. |
| 4 | `dccad8ab64c4e88f7a910ee0826308ece03fdc09` | 76 | 76 | 91 | 89 | Change `while (remaining > 1)` to `for (; remaining > 1; remaining--)` and remove the body decrement. |
| 5 | `c22cb24493b402d555edf7accc9c538f85a55f1d` | 76 | 76 | 91 | 89 | Wrap the loop in `if (remaining > 1)` and use a guarded `do { link load; pointer update; remaining--; } while (remaining > 1)`. |
| 6 | `fbcaa51e81994a266d0512ff12cb814b4a8c5704` | 64 | 64 | 96 | 94 | Replace the head pointer ternary/null check with `if (offset == 0) selection=0; else { node=owner+offset*4;` and retain the existing else body. |
| 7 | `0acd19d0806f6fc324af4ef9e8e027f5a9d01dcb` | 76 | 76 | 92 | 90 | Replace the next-pointer ternary with explicit `if (offset == 0) node=0; else node=owner+offset*4;`. |
| 8 | `5485c531ca4defe0e5b72bbe94f21cc725dc0fde` | 76 | 76 | 91 | 89 | Add `u8 *base=(u8 *)owner`, route both owner-relative pointer calculations through `base`, and use `node-base` for the final difference. |
| 9 | `d7d2de06cda02d5237f488cfa854fb9b1862efd0` | 76 | 76 | 91 | 89 | Cast `fields->count >> 1` to `u16` before converting it to `int`. |
| 10 | `0fee24f6cb7a680a6c63a5c208925e29301a3f37` | 76 | 76 | 91 | 89 | Add an immediate `return` after storing zero for a null head. |
| 11 | `1f93a5b84469bb90001be176136a2807111626e2` | 76 | 76 | 91 | 89 | Use `u8 *node`; cast to `AllocatorNodePacked *` only for `node->next`, retaining byte-pointer difference for final narrowing. |
| 12 | `7ff6f8996e6098f0a12eef2339a50493deb9286d` | 72 | 72 | 97 | 95 | Replace the final selection ternary with explicit null and nonnull branches. |


The ordinary-struct baseline passed host behavior fixtures, but those runs did not validate packed layout assumptions. Its Thumb results are superseded by the genuine packed candidate evidence below.

### Reproducing the current candidate Thumb differential

From the repository root, compile the retained candidate. `probe.py` exits 1 because the candidate is deliberately DIFF; it still writes `build/probes/allocator_recenter/candidate.bin` for the runner. The current candidate symbol is 98 bytes; `original_function_size` remains 0x66 from the independently mapped original boundary. Then generate and run one scenario per count:

```sh
python3 tools/decomp/probe.py src/nonmatching/allocator_recenter.c --start 0x6d38 --end 0x6da0 || test $? -eq 1
python3 - <<'PY'
from pathlib import Path
import json, subprocess
root = Path.cwd()
candidate = (root / "build/probes/allocator_recenter/candidate.bin").read_bytes()
initial = bytearray(0x4000)
for i in range(700):
    word_offset = 8 + i * 4
    byte_offset = word_offset * 4
    next_offset = word_offset + 4 if i < 699 else 0
    initial[byte_offset + 10:byte_offset + 12] = next_offset.to_bytes(2, "little")
for count in (0, 1, 2, 3, 4, 6, 600, 1398):
    memory = bytearray(initial)
    memory[0:2] = (8).to_bytes(2, "little")
    memory[2:4] = (0xBEEF).to_bytes(2, "little")
    memory[4:6] = count.to_bytes(2, "little")
    scenario = {
        "rom_path": str(root / "baserom.gba"),
        "entry_offset": "0x6d38",
        "overlay_offset": "0x6d38",
        "original_function_size": "0x66",
        "candidate_hex": candidate.hex(),
        "args": ["0x02000000"],
        "ram_regions": [{"address": "0x02000000", "size": "0x4000",
                         "initial_hex": memory.hex(), "label": "owner-buffer"}],
        "watch": [{"address": "0x02000000", "size": "0x4000",
                   "label": "whole-owner-buffer"}],
        "compare_registers": ["r4", "r5", "r6", "r7", "sp"],
        "instruction_budget": 10000, "timeout_us": 500000,
    }
    path = Path("/tmp") / f"allocator-recenter-{count}.json"
    path.write_text(json.dumps(scenario))
    subprocess.run(["tools/.deps/thumb-runner/bin/python",
                    "tools/decomp/thumb_runner.py", str(path)], check=True)
PY
```

The node chain stores its next word offset at byte `node+10`; node addresses are `owner + word_offset*4`. The runner validates the USA ROM SHA-1. Its JSON status was `MATCH` for all eight counts, including whole-buffer and preserved-register comparisons.

## Genuine packed-field compiler-shape trials (2026-10-12)

The corrected view declarations are in `include/allocator_packed.h` and use GCC's `__attribute__((packed))` syntax, consistent with `include/unit_record14.h` and `include/job.h`. Target-side `sizeof` assertions require owner size 6 and node size 12. The host harness independently asserts owner offsets head=0, selection=2, count=4, node next=10, and the same packed sizes with `offsetof` and `sizeof`.

Before varying C shapes, I inspected the preprocessed candidate: both declarations retained `__attribute__((packed))` and both size assertions compiled. The pinned GCC2.9 assembly for the packed baseline used `ldrb` at owner offsets 0, 1, 4, 5 and node offsets 10, 11, plus `strb` at owner offsets 2 and 3. It emitted no `ldrh` or `strh` for these fields. This confirmed the field-access lowering rather than inferring packing from the typedef names.

The packed baseline translation unit had source SHA-1 `fafb183b9e6b9b943545d044516e0de87db784a9`:

```c
#include "allocator_recenter.h"
#include "allocator_packed.h"
void AllocatorRecenter(void *owner)
{
    AllocatorOwnerPacked *fields = (AllocatorOwnerPacked *)owner;
    AllocatorNodePacked *node;
    u16 offset;
    int remaining;
    offset = fields->head;
    node = offset ? (AllocatorNodePacked *)((u8 *)owner + (u32)offset * 4) : 0;
    if (node == 0) {
        fields->selection = 0;
    } else {
        remaining = (int)(fields->count >> 1);
        while (remaining > 1) {
            offset = node->next;
            node = offset ? (AllocatorNodePacked *)((u8 *)owner + (u32)offset * 4) : 0;
            remaining--;
        }
        fields->selection = node ? (u16)((u32)((u8 *)node - (u8 *)owner) >> 2) : 0;
    }
}
asm(".align 2,0");
```

Each trial below was compiled at `-O2` with the pinned compiler, and all twelve passed the host harness including packed layout assertions and whole-buffer behavior checks. `Interval diff` is the top-level JSON difference over candidate block versus the 104-byte original interval; `Symbol diff` is the function-entry difference over the candidate and original symbols. Trial 3 is the best packed shape and is now the retained candidate in `src/nonmatching/allocator_recenter.c`.

| Trial | Source SHA-1 | Symbol | Block | Interval diff | Symbol diff | Transformation |
|---|---|---:|---:|---:|---:|---|
| 1 | `0c45d01c9e6e15407283899d1c84707d4f214cf9` | 100 | 100 | 66 | 64 | Replace each owner-relative `* 4` scale with parenthesized `<< 2`. |
| 2 | `0401a4fc3b5938853e27c23434652abaf65dc7b8` | 106 | 108 | 71 | 69 | Replace final unsigned pointer-difference shift with division by 4. |
| 3 | `adf96f1b097903b330e1ced02b80422741331017` | 98 | 100 | 58 | 56 | Change loop to `while (--remaining > 0)` and remove the body decrement. |
| 4 | `c6c856ca530466b954c6cd39b32ae9ce8abee0d4` | 100 | 100 | 66 | 64 | Change loop to `for (; remaining > 1; remaining--)` and remove the body decrement. |
| 5 | `2e50f7d3b1b8b55cbf75a0ccbb296e11c9147fcf` | 100 | 100 | 66 | 64 | Guard a do/while loop with `remaining > 1`. |
| 6 | `34a25446050602e0b0b97e07f7b249761d7b6746` | 90 | 92 | 94 | 92 | Replace head pointer ternary/null test with an offset-zero `if/else`. |
| 7 | `689b5839b7dff7773bfe0de06564a722dbc64e70` | 96 | 96 | 94 | 92 | Replace next-pointer ternary with an explicit offset-zero `if/else`. |
| 8 | `955e4137959133003e821f54e41999939a41120f` | 100 | 100 | 66 | 64 | Add a byte base local and use it for both pointer construction and subtraction. |
| 9 | `441cb71af890b225200855673054a23f17c363c5` | 100 | 100 | 66 | 64 | Cast shifted count to `u16` before converting to `int`. |
| 10 | `d7e5dcb2a9975ed01fa0e0824ed703b70f73f1dd` | 100 | 100 | 66 | 64 | Return immediately after storing zero for a null head. |
| 11 | `08b1437ce67c49863efb7f8fbab9f729d0975e87` | 100 | 100 | 66 | 64 | Use a byte pointer for node and cast only when loading packed `next`. |
| 12 | `0087fe1c7a076ba8d862894e1ae82dc3eea58f79` | 96 | 96 | 93 | 91 | Replace final selection ternary with explicit null/non-null branches. |

The retained packed candidate has source SHA-1 `adf96f1b097903b330e1ced02b80422741331017`, symbol size 98 (original 102), emitted block size 100 (original interval 104), interval diff 58, and symbol diff 56. Its original-vs-candidate Thumb runs matched the whole 16 KiB owner memory and r4–r7/SP for counts 0, 1, 2, 3, 4, 6, 600, and 1398. These checks establish bounded behavior and layout evidence, not byte recovery or integration credit.
