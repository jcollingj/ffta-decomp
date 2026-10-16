# Linked-list node layout

Two original USA Thumb routines establish the three target words in
`ListCountNode`. This document records field placement only; it does not
recover or replace either routine.

The unlink routine at ROM file offsets `[0xC7A34, 0xC7A5C)` loads the removed
node's word at `+4` as its predecessor and its word at `+8` as its successor.
When the predecessor is null, it writes the successor to the owner head at
`+4`; otherwise it writes the successor to `previous + 8`. If a successor
exists, it writes the predecessor to `next + 4`. The routine then frees the
node. This independently identifies the links at `+4` and `+8`.

The append routine at `[0xC7A74, 0xC7AB4)` passes allocation size 12, stores its
payload at node `+0`, and clears node `+8` and `+4`. With an empty owner it
stores the node as the head at owner `+4`. Otherwise it follows links at
`+8` to the tail, stores the new node there, and sets the new node's `+4` to
the tail. This confirms a 12-byte target node with payload/unknown word at
`+0`, previous at `+4`, and next at `+8`.

The source declaration preserves `unknown00` as `u32` because its semantics
are not established by this evidence. `previous` and `next` are typed node
pointers. The pinned target compiler uses four-byte pointers, so the struct is
12 bytes. Native host builds use native pointers and alignment; on a 64-bit
host the expected layout is `unknown00` at 0, `previous` at 8, `next` at 16,
and total size 24. Host harnesses check those native offsets. The host layout
does not describe the ROM representation and must not be serialized as target
bytes.

The owner remains the existing shared `struct Container`. `TreeCountNode`'s
separate `unknown04` field and raw two-word list record APIs are outside this
layout finding. No append/unlink C implementations or matching-function
credit are claimed here; the original routines remain copied code.
