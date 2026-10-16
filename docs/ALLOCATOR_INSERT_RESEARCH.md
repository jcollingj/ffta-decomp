# Allocator insertion research

## Original bounds and evidence

The USA ROM is the expected 16 MiB image with SHA-1
`4ac05441f4de70a4ec3dd932116346c61b8783d9`. Raw Thumb decoding of
`baserom.gba[0x6E28:0x6EC0]` shows a prologue at `0x6E28`, the final return
instruction at `0x6EBC`, and two zero bytes at `0x6EBE..0x6EC0`. The following
symbol `AllocatorInternalAllocate` starts at `0x6EC0` in the supplied
`build/verified-272-recenter-ref.elf`. The candidate symbol bounds are therefore
`[0x6E28, 0x6EBE)` (150 bytes), with a 152-byte probe interval through the next
symbol. Symbol-byte SHA-1: `738dd1b9b78152cf45c6a1d94e36900a5b1e8dcc`.

The original pinned ELF SHA-256 is
`882601940bd20c955372195d628bc2876e8421fb61fd3377aeddd98396bb90bc`. Raw
instructions call `AllocatorFindNeighbors` at `0x6E3E` (Thumb target
`0x08006C89`) and `AllocatorRecenter` at `0x6EB2` (Thumb target
`0x08006D39`). Neither dependency was named in that ELF. With a parent-granted
alias lease, a typed Thumb alias for `AllocatorFindNeighbors` was added to
`asm/external_symbols.s`; its resulting immutable matching reference snapshot
is `build/verified-272-search-ref.elf`, SHA-256
`831b31e5d45d948f9b300d7cca015712c07cb97df0b0b13edbffcad5598c601a`. The
existing `AllocatorRecenter` alias was retained. The alias points at original
copied ROM code; neither dependency was replaced by this candidate.

The allocator records use u16 values stored little-endian. Owner fields are
head offset at `+0`, selected/recentered offset at `+2`, and node count at `+4`.
Node links are previous offset at `+8` and next offset at `+10`; all links are
four-byte word offsets relative to the owner base, with zero as null. The node
size is the u16 at `+6`. The insert helper calls the search helper with that
size, receives previous in output A and next in output B, splices the block,
increments the u16 owner count, then calls recenter.

## Explicit control-flow map before C

| ROM address | Edge or action | Consequence |
| --- | --- | --- |
| `0x6E28..0x6E3C` | Entry saves owner and block; loads block size at `+6`; passes owner, size, and two stack outputs to search. | Search is called exactly once before any mutation by this routine. |
| `0x6E3E` | `BL 0x6C88` | Output A is the previous node; output B is the next node, as confirmed by `AllocatorFindNeighbors` source and the stores below. |
| `0x6E42` | Compare block pointer with zero; nonzero falls through to `0x6E46`; zero branches to `0x6E4E`. | Nonzero path computes `(block - owner) / 4` narrowed to u16. Null path sets the local block offset to zero, but later stores still dereference the block; null is not a supported safe input. |
| `0x6E4C` / `0x6E4E` | Nonzero path joins null path at `0x6E50`. | The narrowed block word offset is used for neighbor links. |
| `0x6E54` | Load previous from output A. If nonnull, branch to `0x6E5A`; if null, branch to `0x6E62`. | A previous node gets its next link set to the new block; otherwise owner head is set to the new block. |
| `0x6E60` | Unconditional branch to `0x6E68`. | Both previous-present and new-head cases continue at next update. |
| `0x6E68` | Load next from output B; if nonnull, fall through to `0x6E6E`; if null, branch to `0x6E74`. | A next node gets its previous link set to the new block. A null next has no neighbor-side update. |
| `0x6E74` | Load previous again; if nonnull, branch to `0x6E7A`; if null, branch to `0x6E82`. | Compute previous's word offset, or zero, and store it in block `+8`. |
| `0x6E8A` | Load next again; if nonnull, branch to `0x6E90`; if null, branch to `0x6E98`. | Compute next's word offset, or zero, and store it in block `+10`. |
| `0x6EA0..0x6EAE` | Read owner count, add one, store low and high bytes. | Count increments modulo 65536. |
| `0x6EB0..0x6EB2` | Pass owner and call recenter. | Recenter runs after all list/count mutations, exactly once. |
| `0x6EB6..0x6EBC` | Restore stack/registers and return. | Single normal return. |

Thus there is no early return in the insertion helper. Its only conditional
edges select whether to update owner head or previous's next link, whether to
update a nonnull next node, and whether each pointer-to-offset conversion
produces zero. The helper does not validate that the supplied block is absent
from the list or that the neighbor outputs are consistent.

## Retained candidate and semantic checks

The descriptive candidate name is `AllocatorInsertFreeBlock`; no original
symbol name has been recovered. Its source is
`src/nonmatching/allocator_insert.c` (SHA-1
`31628421323e1ac4ea72951af9c52476004ceecf`). It represents the block size as
a packed header with a six-byte prefix and u16 size. Target-side typedef checks
require the prefix and complete header sizes to be 6 and 8 bytes; the host
harness independently requires `offsetof(size) == 6` and `sizeof == 8`. The
remaining packed links are accessed as bytes, while dependency outputs are
stored through compatible `void *` temporaries before any byte-pointer cast.

The focused test
`python3 -m unittest tests/test_allocator_insert_c.py -v` passes. Its fixed
whole-buffer expectations cover an empty list, insertion at the head, insertion
between two nodes, and insertion at the tail. Fixtures include word offsets
`0x0100` and above, sizes `0xBEEF`, `0x80F1`, and `0xCAFE`, count wrap from
`0xFFFF` to zero, and count `0xABCD` to `0xABCE`. The search stub asserts the
owner, full u16 size, exactly-once call order, and byte-for-byte unchanged input.
The recenter stub asserts exactly-once call order and compares every byte at
entry against an independently mutated expected image; it then changes only
the selected-offset field and the harness checks the final full buffer. These
are semantic checks for the candidate with controlled dependencies, not
execution of the original target helpers.

The pinned target probe command was:

```sh
python3 tools/decomp/probe.py src/nonmatching/allocator_insert.c \
  --start 0x6e28 --end 0x6ec0 \
  --reference-elf build/verified-272-search-ref.elf
```

It emits a 152-byte block against the 152-byte bounded ROM interval, with 90
differing bytes. The candidate symbol occupies 152 bytes from `0x6E28`; the raw
original function occupies 150 bytes through `0x6EBE`, followed by two zero
alignment bytes. The report has no reviewed function-bound entry, so the
candidate remains an honest `DIFF` and is not matching recovery credit. Its
first 40 bytes match the original entry through the call to search and the
initial block-offset calculation. No instruction injection, register binding,
or compiler-option change was used.

## Bounded compiler-shape experiments

Twelve distinct natural C shapes were compiled with the pinned GCC 2.9 probe
and all twelve passed the same host whole-buffer harness. The table reports
the emitted probe block length and differing bytes against the complete
152-byte bounded interval; it is a byte-difference diagnostic, not a match
claim. Hashes are SHA-1 of the exact C source used for each probe.

| Shape | Source SHA-1 | Candidate block | Differing bytes |
| --- | --- | ---: | ---: |
| Byte-indexed size, separate previous/next pointers, signed `/ 4` offsets | `9b1319dca9076e9995c726c7ada8b7d0df7caab2` | 164 | 128 |
| Same byte-indexed shape with explicit unsigned `((delta << 14) >> 16)` narrowing | `b34015bb08dad331cedf37da928dfc84d659066a` | 144 | 108 |
| Same byte-indexed shape with readable unsigned `delta >> 2` narrowing | `4c8b14c441bd9dc5bd50cc7da5536c4c5b80c917` | 144 | 108 |
| Count increment written `count = count + 1` | `b0e9084688d97e5a88b27e45f1a953769f4f77a1` | 164 | 128 |
| Size decode split into a separate assignment | `4256f292817b640ca0f77901babe8ae0702e84cf` | 164 | 128 |
| Dependency calls use the original `owner` parameter | `d1c84b76cc3e2725337c8c2537a8e307d0775023` | 164 | 146 |
| Block offset computed before the search call | `72c7d5cc2740586ee5eabc1141d6ed5b7f66794c` | 164 | 146 |
| Previous/next local declarations reversed | `90b6890a224de275064fe82bbf589066112d9dbe` | 164 | 128 |
| Packed owner and packed node fields (`u16` at owner 0/2/4 and node 6/8/10) | `e02e3a44d713d1a6b2d06a624c0d31b361609314` | 148 | 129 |
| Packed owner fields with byte-indexed nodes | `f7bbfdae631b9b49dd72296ab144a6cf4bff5558` | 148 | 129 |
| Byte-indexed owner with packed node fields | `9249c7d0a34d87104c2af74ab545adb7c3188ad5` | 148 | 129 |
| Packed size-only header, compatible raw outputs, and unsigned `delta >> 2` | `31628421323e1ac4ea72951af9c52476004ceecf` | 152 | 90 |

The first eight variants are reconstructable from this exact byte-indexed
baseline source. For row two or three replace all three `/ 4` offset
expressions with the table's stated widened expression. Rows four and five
change only the count increment or size-load declaration. Row six changes both
dependency calls from `base` to `owner`. Row seven moves the block-offset
assignment immediately before search. Row eight swaps the declaration groups
for `previous/next` and `previousRaw/nextRaw`.

```c
#include "allocator_insert.h"

#include "allocator_recenter.h"
#include "allocator_search.h"
#include "types.h"

void AllocatorInsertFreeBlock(void *owner, void *block)
{
    u8 *base = (u8 *)owner;
    u8 *node = (u8 *)block;
    u8 *previous;
    u8 *next;
    void *previousRaw;
    void *nextRaw;
    u16 blockOffset;
    u16 offset;
    u16 count;
    u16 size = (u16)((u16)node[6] | ((u16)node[7] << 8));

    AllocatorFindNeighbors(base, size, &previousRaw, &nextRaw);
    previous = (u8 *)previousRaw;
    next = (u8 *)nextRaw;
    blockOffset = node != 0 ? (u16)((node - base) / 4) : 0;
    if (previous != 0) {
        previous[10] = (u8)blockOffset;
        previous[11] = (u8)(blockOffset >> 8);
    } else {
        base[0] = (u8)blockOffset;
        base[1] = (u8)(blockOffset >> 8);
    }
    if (next != 0) {
        next[8] = (u8)blockOffset;
        next[9] = (u8)(blockOffset >> 8);
    }
    offset = previous != 0 ? (u16)((previous - base) / 4) : 0;
    node[8] = (u8)offset;
    node[9] = (u8)(offset >> 8);
    offset = next != 0 ? (u16)((next - base) / 4) : 0;
    node[10] = (u8)offset;
    node[11] = (u8)(offset >> 8);
    count = (u16)((u16)base[4] | ((u16)base[5] << 8));
    count++;
    base[4] = (u8)count;
    base[5] = (u8)(count >> 8);
    AllocatorRecenter(base);
}

asm(".align 2,0");
```

Rows nine through eleven share this packed-field source form:

```c
typedef struct __attribute__((packed)) {
    u16 head;
    u16 selected;
    u16 count;
} InsertOwner;
typedef struct __attribute__((packed)) {
    u8 prefix[6];
    u16 size;
    u16 previous;
    u16 next;
} InsertNode;
```

For row nine, use both records and access `head`, `count`, `size`, `previous`,
and `next` as packed u16 fields. Row ten retains the packed `InsertOwner` but
uses byte-indexed node reads/writes. Row eleven uses byte-indexed owner fields
and packed `InsertNode` fields. All three retain compatible `void *` raw output
temporaries before casting to byte pointers. The final row narrows the packed
node type to only the six-byte prefix and u16 size field; it keeps link/count
accesses byte-indexed, keeps raw outputs as `void *`, and uses unsigned
`delta >> 2`. The table hash is the retained C source hash; the packed type is
defined in `include/allocator_insert.h` (SHA-1
`729b147f8fb784a3aea4c9c6df679da8c89e1df9`). The retained source also has
non-emitting target checks that the packed header and prefix sizes are 8 and 6
bytes. Together these assertions guarantee that the u16 size field starts at
offset 6 under the packed declaration, and the host harness independently
checks its `offsetof` value.

The packed-size shape improved the entry sequence but remains far from a full
match. Further allocator compiler-shape search is outside this bounded
candidate task; semantic correctness remains the priority.
