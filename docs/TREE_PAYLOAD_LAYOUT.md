# Native tree payload layout

## Evidence and field meaning

The tree's first data word remains an integer key. `InsertTreeNode` compares
that word as a signed `int`; the key is still stored in `unknown00` as `u32`.
The second word is a generic payload and can be a native pointer.

The callback at ROM `0x9AC00..0x9AC20` calls `CopyListOwnerWords` at
`0xC7B78`, receives `word4` and `word0` in separate stack slots, and passes
`word4` as a `void *` payload through the callback pair. The later caller at
`0x9AC6C..0x9AC88` gives an entry pointer and owner to `TreeSearchByWord1`,
then passes the found node, entry pointer, and a numeric key loaded from
`entry + 0x24` to `SetListWords`. Together these calls establish that the
second tree word stores and searches for an entry pointer while the first word
continues to hold the numeric key.

## Native representation

`TreeCountNode.unknown00` remains `u32`; `unknown04` is `ListPayload`, the
existing native-width opaque integer used for pointer-bearing content payloads.
The tree links remain native `TreeCountNode *` fields and flags remain `u32`.
`CopyListOwnerWords` and `SetListWords` now take a typed tree node, read or
store the payload before the first numeric word, and use `ListPayload` for the
second word. `TreeSearchContext.key` and `TreeSearchByWord1` use `ListPayload`
so the search query keeps all pointer bits. `InsertTreeNode` also accepts
`ListPayload` while retaining its numeric `int key` and 24-byte target
allocation request.

Target layout and instructions remain unchanged: key offset 0, payload offset
4, links at 8/12/16, flags at 20, and 24 total bytes. The LP64 host layout is
key offset 0, payload offset 8, links at 16/24/32, flags at 40, and 48 total
bytes. The gap after the host key is alignment padding. `include/tree_count.h`
asserts both supported layout families.

## Tests and alias limits

The host tests compile accessor, callback, wrapper, setter, and insertion
production sources as separate translation units. They compare full native
node/context images, cover numeric payloads 0, `0x80000000`, and
`0xFFFFFFFF`, and search two non-dereferenced LP64 payload values with the same
low 32 bits but different upper bits. A separate fixture stores and recovers an
actual retained object pointer. Insertion and setter fixtures verify pointer
identity while keeping the numeric key and allocator size assertions.

`CopyListOwnerWords` preserves its original target store sequence: payload
first, numeric key second. On LP64 the two outputs have different widths, so
tests use correctly typed source-field aliases and a union-backed shared output
with a fixed expected image. They do not cast a `u32 *` to `ListPayload *` to
simulate target cross-width aliasing. The unchanged target bytes provide the
target store-order evidence; the LP64 harness documents the limit of its
different native layout.

The focused tree and accessor tests passed on the LP64 host after the
migration. The harness's ILP32 payload branch was statically audited but was
not executed as a 32-bit host test. Bounded probes
against immutable `build/verified-300-content-ref.elf` (SHA-256
`b509bd395e699edb5597f9ce97befea53b2d941efdf41a027995daeaffc88751`)
matched the original blocks exactly: `CopyListOwnerWords` is 10/10 bytes,
`TreeSearchByWord1` is 44/44, `SetListWords` is 6/6, and
`ListSearchCallback` is 48/48. The full `src/list_accessors.c` block, including
alignment, is 40/40 bytes; each of its three symbols also matched separately.
This type reconciliation adds no callback recovery or ROM/data provenance
credit.
