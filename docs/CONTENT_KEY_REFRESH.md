# Content tree-key refresh

The recovered callback at `0x9AC6C` receives a content-entry payload and its
owning container through the list payload adapter. It searches the owner's
tree for a node whose native payload word equals the entry pointer, writes the
entry pointer back to that node, and copies the entry's numeric word at target
offset `0x24` into the node's key. The callback returns one. The original
callback has no null checks between search and setter; the recovered source
preserves that behavior.

The entry's word at `+0x24` is represented as `u32 unknown24`. The opaque spans
before and after it remain fixed at `0x16` and `0x34` bytes. On the target ABI,
the key and the existing `value5c` stay at `+0x24` and `+0x5C`; on LP64 hosts
they move to `+0x28` and `+0x60` as the native pointer expands.

The three wrappers at `0x9A018`, `0x9A044`, and `0x9A070` choose the owner at
context offsets `0x10`, `0x14`, and `0x18`. Each passes that owner as both the
visitor owner and callback context, using `ContentPayloadCallbackAdapter` with
`VisitLinkedNodes`. After traversal, each reloads its context field and calls
the copied routine at `0xC7C48`, exposed through the typed
`RebuildTreeByNumericKey` alias. Its natural-source candidate remains
nonmatching and receives no matching C credit.

`tests/test_content_key_refresh_c.py` links production sources as separate
translation units. One suite composes the actual callback, payload adapter,
list visitor, tree search, and setter against fixed whole-fixture expectations,
including native pointer payloads and high-bit numeric keys. A separate link
variant substitutes a controlled callback that changes the context owner
during traversal; its rebuild stub verifies the wrapper reloads the owner
after the callback. That controlled mutation test does not imply the recovered
callback itself can perform arbitrary context mutation.
