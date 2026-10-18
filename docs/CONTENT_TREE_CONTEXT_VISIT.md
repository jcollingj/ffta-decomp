# Context-selected tree payload visitors

The six wrappers at ROM offsets `0x99894`, `0x998B0`, `0x998CC`, `0x998E8`,
`0x99904`, and `0x99920` visit the tree owners stored in
`ListCountContext` at offsets `0x10`, `0x14`, and `0x18`. The first three
call `TraverseTreeInOrder` at `0x080C7D18`; the final three call
`TraverseTreeReverseInOrder` at `0x080C7D70`.

Each wrapper creates the original eight-byte callback pair on its stack:
the opaque callback context is stored at pair offset 0, and the payload
callback pointer at pair offset 4. It passes the tree adapter address
`0x0809AC01` to the traversal. This literal is the Thumb entry for
`ContentTreePayloadCallbackAdapter`; it is separate from each wrapper's BL
destination. The adapter reads the tree node payload through
`CopyListOwnerWords`, then invokes the callback stored in the pair.

The six source functions at `src/content_tree_context_visit.c` preserve their exact 28-byte target bounds and
compose with the actual tree adapter, payload accessor, and both traversal
implementations in separate translation units. The host harness gives each
context field a distinct three-node tree. It checks fixed forward and reverse
payload orders for every wrapper, empty trees, complete traversal with
negative nonzero callback results, early stop after two calls, and unchanged
context, owner, node, and callback state outside the explicit callback
effects.

Run the focused composition check with:

```sh
python3 -m unittest tests.test_content_tree_context_visit_c -v
```

The isolated agbcc probe matched the complete 168-byte block in
`[0x99894, 0x9993C)`. Each of the six candidate symbols is 28 bytes and starts
at its original address. The linker layout now emits the copied suffix from
`0x9993C` through its prior end at `0x99CB8`.
