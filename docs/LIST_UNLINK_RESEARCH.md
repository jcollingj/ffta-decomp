# List unlink-and-free research

## Bounds and raw instructions

The candidate interval is `[0xC7A34, 0xC7A5C)`, exactly 40 bytes. I checked
the boundary independently against `baserom.gba`: offset `0xC7A34` begins with
`00 b5` (`push {lr}`), and the interval ends with `00 47` (`bx r0`) at
`0xC7A5A`. The next halfword is outside the interval. The ROM source hash is
the project USA SHA-1 `4ac05441f4de70a4ec3dd932116346c61b8783d9`.

The raw disassembly command was:

```sh
arm-none-eabi-objdump -D -b binary -m arm \
  --start-address=0xC7A34 --stop-address=0xC7A5C \
  -M force-thumb baserom.gba
```

All addresses below are ROM file offsets. The sequence is:

| Address | Instruction | Effect |
| --- | --- | --- |
| `C7A34` | `push {lr}` | Save return address. |
| `C7A36` | `adds r3, r0, #0` | Retain `owner`. |
| `C7A38` | `ldr r2, [r1, #4]` | Load `node->previous`. |
| `C7A3A` | `cmp r2, #0` | Select head or non-head path. |
| `C7A3C` | `bne C7A44` | Non-null predecessor branches to predecessor update. |
| `C7A3E` | `ldr r0, [r1, #8]` | Load `node->next`. |
| `C7A40` | `str r0, [r3, #4]` | Set `owner->list_head`. |
| `C7A42` | `b C7A48` | Join before the successor reload. |
| `C7A44` | `ldr r0, [r1, #8]` | Load `node->next`. |
| `C7A46` | `str r0, [r2, #8]` | Set `previous->next`. |
| `C7A48` | `ldr r2, [r1, #8]` | Reload `node->next` after the previous-link update. |
| `C7A4A` | `cmp r2, #0` | Test the successor. |
| `C7A4C` | `beq C7A52` | Null successor skips its previous-link write. |
| `C7A4E` | `ldr r0, [r1, #4]` | Reload `node->previous`. |
| `C7A50` | `str r0, [r2, #4]` | Set `next->previous`. |
| `C7A52` | `ldr r0, [r3, #0]` | Load `owner->allocator_owner`. |
| `C7A54` | `bl 0x7170` | Call verified `AllocatorFree(owner->allocator_owner, node)`. |
| `C7A58` | `pop {r0}` | Restore return address. |
| `C7A5A` | `bx r0` | Return; symbol ends at `C7A5C`. |

This establishes both mutually exclusive predecessor paths and the successor
reload/write order directly. The behavior source uses the recovered native
`ListCountNode.previous` pointer from `include/list_count.h`, the shared
`struct Container`, and the existing allocator API. It adds no guards and
does not clear the removed node's own links.

## Candidate and verification

The recovered source is `src/list_unlink.c` with declaration in
`include/list_unlink.h`. The isolated probe command was:

```sh
python3 tools/decomp/probe.py src/list_unlink.c \
  --start 0xc7a34 --end 0xc7a5c \
  --reference-elf build/verified-273-list-free-ref.elf --disassemble
```

The selected immutable reference ELF has SHA-256
`36f8de2cb35cd5d54bab32be1851db3e9c647e2abacdcca21667da5a87a03eb7`.
The candidate ELF independently reports `UnlinkAndFreeListNode` at
`0x080C7A34`, size 40 (`nm -S`). The probe reports a 40-byte candidate block,
40-byte original block, and zero whole-block differences. Its comparison JSON
initially labeled the symbol `UNINVENTORIED` because the function bound had not
yet been added to a manifest; after review, the function and matching C region
were admitted at the exact address and length. The original 40-byte block SHA-256 is
`2995ade8fcd3bb09bea2cb99528e02e023bd83afc25fa892c548af3e6840f660`.

Integration split the former copied region at `0xC7A5C`: the recovered
40-byte function occupies `[0xC7A34,0xC7A5C)`. After `AppendListNode` was
integrated, the copied remainder was split into
`[0xC7A5C,0xC7A74)` in `asm/rom_list_append_prefix.s` and
`[0xC7AB4,0xC7B08)` in `asm/rom_list_append_suffix.s`. The prefix retains the
`rom_list_unlink_suffix_start` boundary symbol used by the linker assertions.
The prior `asm/rom_tree_clear_suffix.s` wrapper was
retired. The Makefile and region ledger now reference the new object/source;
the linker checks the function start and the suffix boundary, which fixes the
symbol size at 40 bytes.

`tests/test_list_unlink_c.py` compiles the harness and candidate as separate
translation units. The retained-allocation `AllocatorFree` stub asserts the
exact full owner and node arrays at call time, allocator-owner and allocation
pointer identity, and exactly one call. Fixed sole, head, middle, and tail
fixtures distinguish both predecessor branches and both successor-presence
paths. The complete expected node image confirms that the removed node's own
links and all unrelated fields remain unchanged; owner allocator and tree
root are also checked. Valid list fixtures establish the observable final
state. The intra-function store ordering itself is established by the raw
instruction trace above, since no callback occurs between those stores.

Focused verification:

```text
python3 -m unittest tests.test_list_unlink_c -v
Ran 1 test ... OK
```

`make compare progress` and the focused host test passed after integration.
The function is counted as matching C only after the full matching comparison
and provenance gate; host behavior alone is not the admission evidence.
