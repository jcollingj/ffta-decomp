# Context-field payload visitors

The three wrappers at ROM offsets `0x99840`, `0x9985C`, and `0x99878` visit
the list owners stored at `ListCountContext` offsets `0x10`, `0x14`, and
`0x18`. Each wrapper receives a generic payload callback and its opaque
callback context.

Each original wrapper places an eight-byte callback pair on its stack: the
opaque context at pair offset 0 and callback pointer at pair offset 4. It
loads the selected owner, passes callback address `0x0809ABE9` in the traversal
callback argument, and calls `VisitLinkedNodes` at `0x080C7CC0` with the pair
address as traversal context.

The traversal callback address is an adapter, not the user's payload callback.
At `0x0809ABE8`, the adapter saves the pair pointer, calls `0x080C7B74` to
load the node's word at offset 0 (the payload), then loads pair context and
callback and invokes the payload callback through the call thunk at
`0x0814224C`. `VisitLinkedNodes` walks the owner's head at offset 4 using each
node's next pointer at offset 8 and calls its callback through `0x0814225C`;
a zero result stops traversal and any nonzero result continues.

The recovered wrappers use the shared `ContentPayloadCallbackPair` and
`ListCountContext` declarations and compose with the actual adapter, payload
getter, and linked-node visitor. The host harness compiles each production
source as a separate translation unit. Its expected image is built from
independent fixed fixture values and explicit callback effects; it checks all
three distinct owner lists, empty/single/three-node traversal, zero-stop and
seven-continue outcomes, callback-visible context, and unchanged owners,
nodes, and payloads. The stack pair exists only during the wrapper call and is
never retained by the callback.
