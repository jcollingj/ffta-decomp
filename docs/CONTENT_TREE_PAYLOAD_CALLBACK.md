# Tree payload callback adapter

`ContentTreePayloadCallbackAdapter` recovers the 32-byte function at ROM
offset `0x9AC00`. The inspected bytes have SHA-256
`aed5678af826d6c1dbe5188bcb3617617650bd50d4b39ae9ffbe061f5299371e`.
The unchanged reference ELF `build/verified-304-content-ref.elf` has SHA-256
`f587d2c891e7c75f179e31005a03791ee13a5f77723af88829cf54146f1ef92a`.

The adapter receives a tree node and callback-pair pointer using the generic
tree traversal ABI `int (void *, void *)`. It saves the pair pointer, calls
`CopyListOwnerWords` at `0xC7B78` with separate `ListPayload` and `u32` output
slots, then calls the pair callback with the payload and pair context through
the bridge at `0x14224C`. The callback's signed `int` result is returned
unchanged. Payloads remain native-width pointers; the numeric key output stays
`u32`.

Raw caller instructions in `[0x99894, 0x9993C)` show that the callback literal
is `0x0809AC01` (the Thumb entry for this adapter). The first three wrappers
at `0x99894`, `0x998B0`, and `0x998CC` load owner fields at `+0x10`, `+0x14`,
and `+0x18` and call `TraverseTreeInOrder` at `0xC7D18`. The next three at
`0x998E8`, `0x99904`, and `0x99920` load the same owner fields and call
`TraverseTreeReverseInOrder` at `0xC7D70`. These BL destinations are distinct
from the callback literal.

The focused host harness compiles the adapter, `CopyListOwnerWords`, and both
tree traversal functions as separate translation units. It checks fixed
in-order and reverse-in-order traces over a seven-node tree, distinct numeric
keys, distinct native payload identities including null and duplicates,
forwarded callback context, exact callback results `0`, `1`, `7`, and `-1`, and
zero-result early stopping in both traversal directions. It compares complete
owner, node, callback-pair, and callback-state images; the callback guard is
live state in the tested object. The test command is:

```sh
python3 -m unittest tests.test_content_tree_payload_callback_c -v
```

The isolated agbcc probe against the verified reference matches all 32 bytes
in `[0x9AC00, 0x9AC20)`. The candidate symbol is exactly 32 bytes at
`0x0809AC00`. The source is integrated at `src/content_tree_payload_callback.c`;
the matching build records its 32-byte function and source region separately.
