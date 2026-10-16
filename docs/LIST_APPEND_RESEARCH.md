# Linked-list append recovery

## Bounds and original control flow

The routine occupies `[0xC7A74, 0xC7AB2)` (62 symbol bytes); the requested
block `[0xC7A74, 0xC7AB4)` adds two zero alignment bytes. The source ROM is the
USA image with SHA-1 `4ac05441f4de70a4ec3dd932116346c61b8783d9`.

The raw instruction listing was generated with:

```sh
arm-none-eabi-objdump -D -b binary -m arm \
  --start-address=0xC7A74 --stop-address=0xC7AB4 \
  -M force-thumb baserom.gba
```

The relevant branches and updates are:

| Address | Instruction | Effect |
| --- | --- | --- |
| `C7A74` | `push {r4,r5,lr}` | Preserve callee-saved registers and return address. |
| `C7A76` | `adds r5,r0,#0` | Retain the owner. |
| `C7A78` | `adds r4,r1,#0` | Retain payload. |
| `C7A7A` | `ldr r0,[r5,#0]` | Read allocator owner before allocation. |
| `C7A7C` | `movs r1,#12` | Request 12 target bytes. |
| `C7A7E` | `bl 0x7138` | Call `AllocatorAllocate`. |
| `C7A82` | `adds r2,r0,#0` | Retain returned node. No null check follows. |
| `C7A84` | `str r4,[r2,#0]` | Store payload/unknown word. |
| `C7A86–C7A8A` | zero stores at `+8`, then `+4` | Clear `next`, then `previous`. |
| `C7A8C` | `ldr r0,[r5,#4]` | Reload owner head after the allocator call. |
| `C7A8E–C7A90` | `cmp r0,#0; bne C7A96` | Empty and nonempty paths split. |
| `C7A92` | `str r2,[r5,#4]` | Empty path sets owner head to new node. |
| `C7A94` | `b C7AAA` | Empty path skips tail traversal and linking. |
| `C7A96` | `adds r1,r0,#0` | Nonempty path initializes current tail from the reloaded head. |
| `C7A98` | `b C7A9C` | Initial loop entry skips the back-edge's link load. |
| `C7A9A` | `ldr r1,[r1,#8]` | Back-edge advances to the next node. |
| `C7A9C` | `ldr r0,[r1,#8]` | Check current node's next pointer. |
| `C7AA0` | `bne C7A9A` | Non-null next repeats via the link load; null falls through. |
| `C7AA2` | `str r2,[r1,#8]` | Link old tail to new node. |
| `C7AA4` | `str r1,[r2,#4]` | Set new node's previous pointer. |
| `C7AA6–C7AA8` | zero store at `+8` | Clear new node's next pointer again on nonempty path. |
| `C7AAA–C7AB0` | move result; restore; `bx r1` | Return the allocated node; symbol ends after `bx` at `C7AB2`. |

The `C7A98` branch target is material: the first check is performed on the
head itself, while subsequent checks are reached through `C7A9A` and advance
one link at a time. The owner head read at `C7A8C` also occurs after the
allocator callback, so a callback mutation is visible to the append operation.
The original has no allocation-failure guard.

## Candidate and evidence

`src/list_append.c` uses the existing `Container` and
`ListCountNode` declarations and calls the existing `AllocatorAllocate` API.
The header is `include/list_append.h`. A first natural source shape used the
head variable directly as the loop cursor and produced 33 differing bytes in
the 64-byte block (candidate symbol 58 bytes). Naming a separate `head` and
copying it into `tail` in the nonempty arm caused GCC 2.9 to retain the target
register/control-flow shape. No assembly instructions or compiler settings
were changed. Current source SHA-256:

```text
1f5a1a4dc8937ec104bd2217de998d68541e16c2cf5c0d250ac5746861907a07
```

The isolated comparison command used before integration was:

```sh
python3 tools/decomp/probe.py src/list_append.c \
  --start 0xc7a74 --end 0xc7ab4 \
  --reference-elf build/verified-277-container-ref.elf --disassemble
```

The selected immutable reference ELF SHA-256 was
`f4572a7c8e404ad75864cd1a1c757cae704bba53100e38b471695363a646fa22`.
`nm -S` reports `AppendListNode` at `0x080C7A74`, symbol size `0x3E` (62
bytes). The probe reports 64 candidate and original block bytes, zero
symbol-aligned differences, zero whole-block differences, and `accepted: true`.
The pre-integration probe labeled the symbol `UNINVENTORIED`. Integration
promotes the source and records the matching symbol in the function inventory.

## Host behavior checks

`tests/c/list_append_harness.c` is compiled separately from the candidate by
`tests/test_list_append_c.py`, using `src/list_append.c`. The allocator stub retains native-sized
storage, checks allocator identity and exactly one call, and asserts the
target request is 12 without truncating host pointers. Fixed expected owner,
node-array, alternate-chain, and allocated-node images cover empty, one-node,
and three-node cases; payloads include zero, a high-bit value, and a duplicate
of an existing node payload. The result pointer must be the retained
allocation. An allocator callback replaces `owner.list_head` with a distinct
two-node chain, proving the candidate uses the post-call head value. Whole
images verify unrelated owner and node fields are preserved.

Focused command and result:

```text
python3 -m unittest tests.test_list_append_c -v
Ran 1 test ... OK
```

This host test supports behavioral semantics. The original-byte probe is the
evidence for target code and symbol matching. Session 54 integrated the
matching source and carve. Integration cleanup found that the retired
`asm/rom_list_unlink_suffix.s` path remained as a zero-byte untracked file.
It was deleted and `test ! -e` confirmed its absence. A repository search
found no build, linker, or manifest references to that wrapper; this research
note is the only remaining textual mention documenting its retirement.
