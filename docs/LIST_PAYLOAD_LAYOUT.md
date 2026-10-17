# Generic linked-list payload layout proposal

## Status

The linked-list payload proposal was implemented under the explicit shared-file
lease after parent checkpoint 283. A later tree payload migration is described
in `TREE_PAYLOAD_LAYOUT.md`; it supersedes this document's earlier statement
that `CopyListOwnerWords` and `TreeCountNode` remain numeric and out of scope.

## Finding

The recovered target list node stores one 32-bit word at offset zero followed
by native links at offsets four and eight. `AppendListNode` stores its second
argument there, and `FindListNodeByWord` compares that same word as an unsigned
value. `GetListOwnerWord0` previously loaded and returned that first word. Its
only present caller before this migration was `tests/c/list_accessors_harness.c`; the original
callback at `0x9AB5C` uses the getter result as a pointer. The distinct
callback at `0x9AB7C` reads signed halfwords at offsets `+8` and `+12`. The
callback at `0x9ABB0` follows the pointer in payload word zero and reads bytes
at `+0xF6` and `+0xF7`. A host representation that truncates pointers to `u32`
cannot compose these recovered paths.

At the time of the original list migration, `CopyListOwnerWords` was treated as
a separate two-numeric-word operation. Raw caller evidence later showed that
its second output is a tree payload pointer, so the typed tree-node update is
recorded separately in `TREE_PAYLOAD_LAYOUT.md`.

The complete list-payload inventory found these production declarations and
operations:

- `include/list_count.h`: `ListCountNode.unknown00` is presently `u32`.
- `include/list_append.h` and `src/list_append.c`: append accepts and stores a
  `u32` payload.
- `include/list_find_word.h` and `src/list_find_word.c`: search accepts and
  compares a `u32` word.
- `include/list_accessors.h` and `src/list_accessors.c`:
  `GetListOwnerWord0` reads the payload from a node and returns `u32` before
  this migration;
  `CopyListOwnerWords` is independent and stays as-is.

Current direct production call sites of append, find, and the getter are absent
apart from the declarations/definitions: append and getter are currently
exercised by their harnesses, and numeric find is also exercised by
`tests/c/list_find_set_harness.c` and
`tests/c/container_owner_integration_harness.c`. Numeric assignments to
`unknown00` occur throughout list and tree fixtures, but do not require
algorithm changes. The impacted existing accessor and API harnesses are
`tests/c/list_accessors_harness.c`, `tests/c/list_append_harness.c`, and
`tests/c/list_find_set_harness.c`.

## Recommended representation

Introduce `typedef unsigned long ListPayload` in a small shared
`include/list_payload.h`. Assert at compile time that `sizeof(ListPayload) ==
sizeof(void *)`, that target builds have four-byte `ListPayload`, and that the
compiler uses one of the supported ILP32 or LP64 width combinations. The
project's GCC 2.9 target ABI has 32-bit `unsigned long` and pointers; the host
ABI has 64-bit `unsigned long` and pointers. LLP64 and other unsupported
pointer/integer layouts fail at compile time instead of truncating host
pointers.

Use `ListPayload` for `ListCountNode.unknown00`, the `AppendListNode` payload,
the `FindListNodeByWord` key, and the `GetListOwnerWord0` return value. Give
the getter a `const struct ListCountNode *` input and read its `unknown00`
member directly. Keep the `unknown00` field name so existing numeric fixtures and
consumers remain source-compatible. Pointer-bearing callers explicitly
convert through `unsigned long` when converting between a pointer and
`ListPayload`; there is no host fixture resolver and no cast through `u32`.
This is the normal implementation-defined pointer/integer round trip for the
two supported GCC ABIs, with width checked at compile time.

This representation keeps the original unsigned numeric equality model while
allowing a full native pointer identity as a payload. On the GBA,
`ListPayload` is exactly the old 32-bit word and all field offsets, node size,
loads, stores, comparisons, and target numeric boundary values remain
unchanged. On a 64-bit host, it occupies the same eight bytes as a pointer;
the existing native link offsets (8 and 16) and 24-byte node size therefore
remain unchanged. A union would require every numeric user and pointer user to
select and agree on a member, while introducing inactive-member/aliasing
semantics for the raw original word. A pointer-only member would no longer
preserve the APIs' arbitrary numeric keys. The native-width opaque integer
avoids both problems.

## Exact migration scope

After a separate explicit lease, make only these production changes:

1. Add `include/list_payload.h` with the native-width typedef and C89-compatible
   compile-time assertions.
2. Update `include/list_count.h` to use `ListPayload` for `unknown00` and retain
   the current target node-size assertion.
3. Update `include/list_append.h` and `src/list_append.c` argument types to
   `ListPayload`.
4. Update `include/list_find_word.h` and `src/list_find_word.c` key types to
   `ListPayload`.
5. Update only `GetListOwnerWord0` in `include/list_accessors.h` and
   `src/list_accessors.c` to read/return `ListPayload`; leave
   `CopyListOwnerWords` exactly `u32`-based.

The implementation migrated the existing harnesses to declare payload
inputs/expected values with `ListPayload` where applicable. The new focused
composition harness and Python test compile the real `src/list_append.c`,
`src/list_accessors.c`, and `src/list_find_word.c` as separate translation
units. Their allocator dependency uses a controlled allocator stub; no
production source is included into the composition harness translation unit.

## Composition test oracle

The composition harness uses retained, distinct native objects as pointer
payloads and a fixed independent expected image. After each append, it compares
the complete owner and all linked/guard nodes byte-for-byte against separately
initialized expected storage. It then:

- recovers the appended node's payload with the real `GetListOwnerWord0`,
  convert it back to a pointer, and assert exact identity and a known object
  field value;
- finds each pointer payload through real `FindListNodeByWord`, with distinct
  identities proving no truncation or accidental match;
- appends and finds numeric tokens `0`, `0x80000000UL`, and `0xFFFFFFFFUL`, using
  fixed expected results and whole-state images; and
- includes a host-width numeric boundary token `ULONG_MAX` to prove
  storage/search does not silently narrow to the target word width. On ILP32,
  this intentionally aliases `0xFFFFFFFFUL` and finds its first matching node.

The pointer tokens and numeric tokens must be distinct in the fixture. A
multi-node case should include at least two pointer payloads and a numeric
payload, and verify first-match behavior for a repeated numeric key if retained
by the existing search contract. The expected images should be specified as
fixed field values and actual expected links, not generated by a duplicate
append/search implementation. Include compile-time checks for payload width,
field offset, link offsets, and node size on the host. The existing node layout
and append fixtures provide the baseline for unchanged surrounding state. The
final getter and search calls are bracketed by whole-owner/node snapshots, and
the retained payload objects are checked against their fixed original image
after those read-only calls.

## Matching verification and limits

The matching compiler comparison reports exact original bytes for
`AppendListNode` (62 bytes), `GetListOwnerWord0` (4 bytes), and
`FindListNodeByWord` (30 bytes). The getter remains a single 32-bit load and
return; append stores the argument with a word store, and find compares with a
word load/compare. Target preprocessed declarations show `ListPayload` as
`unsigned long`, and target compilation passes the four-byte node/payload
assertions. The focused composition test plus existing append, accessor,
find/set, and list-count tests pass under the host compiler. No full suite or
integration gate was run as part of this leased task.

This representation unifies types needed by the recovered content callbacks;
it does not itself recover callback code or add function, byte, or ROM/data
provenance credit. The immutable reference build remains
`verified-283-content-ref.elf` with SHA-256
`680c2cffcfacb1ccbd7931d1b7cf388c088fce9776c3490370ffa13a9b6f26f0`.
