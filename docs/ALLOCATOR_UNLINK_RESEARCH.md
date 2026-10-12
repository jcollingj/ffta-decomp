# Allocator free-list unlink research

## Raw routine bounds and control flow

`AllocatorUnlinkFreeBlock` begins at ROM file offset `0x6DA0`. Its final instruction is `bx r0` at `0x6E24`, so the Thumb symbol is `[0x6DA0, 0x6E26)` and is 134 bytes. The following two raw bytes at `0x6E26..0x6E27` are `00 00` alignment; the next routine begins at `0x6E28`. The symbol bytes have SHA-1 `2ee44c999d224bfc092422d8fa2c30f4ee9bb64d`; the complete 136-byte block has SHA-1 `e996288ec3034b0316a607a44f313c7cc6ee4fa4`. The USA base ROM remains SHA-1 `4ac05441f4de70a4ec3dd932116346c61b8783d9`.

The routine receives an allocator owner in `r0` and a block in `r1`. It reads the block's packed 16-bit previous and next word offsets at byte offsets `+8` and `+10`; zero means no neighbor, and nonzero offsets resolve to `owner + (offset << 2)`. It patches the previous block's next offset, or the owner head at `+0` when there is no previous block. It patches the next block's previous offset when present, clears the removed block's next then previous offsets, decrements the 16-bit owner counter at `+4` with 16-bit wrap, then calls the recenter helper.

The call at `0x08006E1C` branches to `0x08006D38`. This is the `AllocatorRecenter` entry. Its Thumb symbol value is `0x08006D39`. Owner field `+2` is called the **selection/recenter offset** here; this routine invokes the helper but does not establish that the field means “tail.” The helper remains outside this candidate and the host fixture stubs it.

## Candidate and host evidence

The candidate source is [allocator_unlink.c](../src/nonmatching/allocator_unlink.c), with its public declaration in [allocator_unlink.h](../include/allocator_unlink.h). The host test is [test_allocator_unlink_c.py](../tests/test_allocator_unlink_c.py), and the packed-buffer fixture and exact helper call-entry assertion are in [allocator_unlink_harness.c](../tests/c/allocator_unlink_harness.c).

Run the focused host test with:

```sh
python3 -m unittest tests.test_allocator_unlink_c -v
```

The fixture keeps the owner and blocks in one 2 KiB byte array. It checks sole, head, middle, and last-block removal; null previous/next links; word offsets `0x0120`, `0x0180`, and `0x01F0`; counter decrement `1 -> 0`; and underflow `0 -> 0xFFFF`. The expected bytes are mutated separately from the actual buffer. At helper entry the stub checks owner identity, one-call ordering, and every byte of the expected post-unlink buffer, then writes only the selected/recenter offset expected for that fixture. The final buffer is compared in full.

The helper declaration comes from `allocator_recenter.h`; the unlink source includes that header. No candidate helper implementation is copied into this test.

## Matching probe evidence

The shared source now declares the verified Thumb alias in `asm/external_symbols.s`. The isolated reference snapshot is `build/verified-272-recenter-ref.elf`, SHA-256 `882601940bd20c955372195d628bc2876e8421fb61fd3377aeddd98396bb90bc`. The snapshot was made read-only after `make compare progress` passed with 272 matching C functions and an unchanged full-ROM hash. It contains the absolute `AllocatorRecenter` FUNC symbol at `0x08006D39`; no session 39 candidate was linked into it.

Probe command:

```sh
python3 tools/decomp/probe.py src/nonmatching/allocator_unlink.c --start 0x6DA0 --end 0x6E28 --reference-elf build/verified-272-recenter-ref.elf
```

Result: **DIFF**, 112 candidate bytes versus the 136-byte original block, with 123 differing block bytes. The candidate symbol is 112 bytes; it is not admitted as matching C. The probe's preprocessed candidate SHA-256 is `977e66ee8943327b5506a4f6d4058522a94d2187969a9e06c237025e174897d6`. After the probe, the direct prototype was replaced by `#include "allocator_recenter.h"` in the same declaration order. Preprocessing the current source produces the same SHA-256, so this cleanup leaves the probed candidate translation unit unchanged.

## Natural-source hypotheses

All eight variants below were compiled with the pinned agbcc probe, linked against the reference alias above, and passed the host packed-buffer fixture. None matched the original block. Candidate source hashes identify the exact preprocessor-independent variant source saved with the original probe outputs; optimization level is a probe setting, not an injected opcode or compiler modification.

| Variant | Natural-source change | Optimization | Symbol/block bytes | Differing block bytes | Source SHA-256 |
| --- | --- | ---: | ---: | ---: | --- |
| `o2_byte_fields` | Baseline explicit little-endian byte reads/writes and conditional offset-to-pointer conversion | 2 | 112 / 112 | 123 | `c6d4e1dd9d6cb58e862b0ce00c1aff8d46e0c91e6e3f7fe4ed85f222fcc714d1` |
| `o1_byte_fields` | Same baseline source | 1 | 116 / 116 | 118 | `c6d4e1dd9d6cb58e862b0ce00c1aff8d46e0c91e6e3f7fe4ed85f222fcc714d1` |
| `o0_byte_fields` | Same baseline source | 0 | 382 / 384 | 380 | `c6d4e1dd9d6cb58e862b0ce00c1aff8d46e0c91e6e3f7fe4ed85f222fcc714d1` |
| `o2_native_records` | Replace byte views with an owner record (`u16 head, tail, count`) and block record (`u8 prefix[8]; u16 previous, next`) | 2 | 94 / 96 | 130 | `5c3cd6c076a10a61e6ce52e4ab1803dd0d586b6a6be250693fa016ef167c396c` |
| `o2_halfword_access` | Read/write links and count through `u16 *` indexes 4/5 and 0/1/2; retain byte pointers for offset resolution | 2 | 76 / 76 | 125 | `2e385ad3c22b12910d392e56198255bfcab5de16c768fa5d12328977e7f614b4` |
| `o2_unconditional_offset_math` | Compute both neighbor pointers even for zero offsets, then branch on the original offsets before dereferencing | 2 | 108 / 108 | 129 | `b9708daa05127b4dde94b616b85cdae6d7388af630455143bbb5caa6daa8a227` |
| `o2_chained_clear` | Chain the four link-byte clears in one assignment and compact the counter read/decrement/write scope | 2 | 112 / 112 | 123 | `43377624177b657e0f54237f5d2847256ac110c03bbd9c9f9bbd9186b9ce183d` |
| `o2_volatile_byte_fields` | Mark owner, node, and neighbor byte pointers volatile | 2 | 118 / 120 | 125 | `64066eee964fc122db5cfb3d1b609871bcbb3c244a0a8e340fc0eac566e9d3ae` |

To reconstruct the variants, begin with the baseline function in `src/nonmatching/allocator_unlink.c`. For `native_records`, replace its locals and body with owner fields `head`, `tail`, `count` and block fields `prefix[8]`, `previous`, `next`, then update neighbors through those fields. For `halfword_access`, replace explicit byte assembly/stores with `((u16 *)owner)[0..2]`, `((u16 *)block)[4..5]`, and matching neighbor indices. For `unconditional_offset_math`, remove the zero ternaries around both `base + (offset << 2)` expressions and test `previousOffset`/`nextOffset` before writes. For `chained_clear`, use `node[8] = node[9] = node[10] = node[11] = 0` and combine counter read/decrement/write in a local scope. For `volatile_byte_fields`, qualify `base`, `node`, `previous`, and `next` as `volatile u8 *`. The first three rows use the identical baseline source with the listed `--opt` setting.

The byte-field `-O2` source is retained because it is clear and passes the independent state fixture; none of these results justifies matching credit or a compiler-specific workaround.
