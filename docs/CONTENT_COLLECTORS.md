# Context content payload collectors

The immutable matching reference used for this recovery was
`build/verified-304-content-ref.elf`, SHA-256
`f587d2c891e7c75f179e31005a03791ee13a5f77723af88829cf54146f1ef92a`.
Raw Thumb disassembly was read with `arm-none-eabi-objdump -D -M force-thumb`.
The original entries begin at each interval start (`push {lr}`), and each
wrapper ends after `bx r1` plus its two 32-bit literal words.

| ROM offset | Direct BL destination | Owner field | Callback literal | Adapter literal |
| --- | --- | --- | --- | --- |
| `0x99CDC` | `VisitLinkedNodes` `0x080C7CC0` at `0x99CF4` | `+0x10` | `0x0809AC21` | `0x0809ABE9` |
| `0x99D34` | `VisitLinkedNodes` `0x080C7CC0` at `0x99D4C` | `+0x18` | `0x0809AC21` | `0x0809ABE9` |
| `0x99D60` | `TraverseTreeInOrder` `0x080C7D18` at `0x99D78` | `+0x10` | `0x0809AC21` | `0x0809AC01` |
| `0x99D8C` | `TraverseTreeReverseInOrder` `0x080C7D70` at `0x99DA4` | `+0x10` | `0x0809AC21` | `0x0809AC01` |
| `0x99DB8` | `TraverseTreeInOrder` `0x080C7D18` at `0x99DD0` | `+0x14` | `0x0809AC21` | `0x0809AC01` |
| `0x99DE4` | `TraverseTreeReverseInOrder` `0x080C7D70` at `0x99DFC` | `+0x14` | `0x0809AC21` | `0x0809AC01` |
| `0x99E10` | `TraverseTreeInOrder` `0x080C7D18` at `0x99E28` | `+0x18` | `0x0809AC21` | `0x0809AC01` |
| `0x99E3C` | `TraverseTreeReverseInOrder` `0x080C7D70` at `0x99E54` | `+0x18` | `0x0809AC21` | `0x0809AC01` |

Every 44-byte entry begins with the same setup: a 16-byte stack frame,
zeroed count at `sp+0`, output cursor copied from `r1` to `sp+4`, pair context
set to the local state at `sp+8`, and callback stored at pair offset `+4`.
The wrapper reloads the count from `sp+0` after traversal. The two words at
the end of each interval are literals loaded by PC-relative `ldr` instructions;
they are not wrapper instructions. In particular, `0x0809AC21` names
`ContentAppendCallback`, while the tree adapter literal is
`0x0809AC01`. The existing list adapter is `0x0809ABE9`.

The already recovered `FillContentConsumerEntries` at
`[0x99D08,0x99D34)` was left unchanged. Its actual entry has the same setup,
loads owner `+0x14`, directly calls `VisitLinkedNodes` at `0x99D20`, and loads
the same callback and list-adapter literals at `0x99D2C` and `0x99D30`.

The source is split into the 44-byte `+0x10` list collector and the remaining
seven 44-byte wrappers in original address order. The latter use the tree
adapter API from `content_tree_payload_callback.h`; list wrappers use the
existing `ContentPayloadCallbackAdapter` API. All wrappers initialize
`ContentAppendState` and `ContentPayloadCallbackPair` directly and return the
append count.

Both bounded isolated probes matched their entire original spans: the first
candidate emitted 44/44 bytes at `0x99CDC`, and the second emitted 308/308
bytes at `0x99D34`. Candidate ELF symbols also show all eight reserved
collector names at their specified starts with size `0x2C` (44) each. The
second probe used a build-only derived reference ELF to resolve the just
integrated tree-adapter symbol. It was made by adding the typed symbol
`ContentTreePayloadCallbackAdapter` at `0x0809AC01` to the immutable reference;
the derived ELF SHA-256 is
`5623fabf5119909bd8e29fd2d5f3160abf3b63d2b86759093dc219f9ce1a8026`. The
unmodified reference hash above remains the source of all other symbol
addresses.

The focused host fixture links each candidate as a separate translation
unit with the real list visitor, both tree visitors, accessors, list and tree
payload adapters, and append callback. Fixed owner-specific payload-index
vectors make list order distinguishable from tree in-order and reverse order;
the fixtures include duplicate and null payloads. Each wrapper runs empty,
single-node, and multi-node cases. Assertions compare the returned count,
entire guarded output buffer (including untouched tail and live sentinels),
and complete context, owner, list-node, and tree-node images. Output capacity
is a caller precondition, matching the original code; no capacity check is
inferred. The focused host test command
`python3 -m unittest tests.test_content_collectors_c -v` passed.
After promotion into the matching build, `make compare` reported all eight
collector symbols as `MATCH` at their original addresses, and retained
`FillContentConsumerEntries` as a 44-byte `MATCH` at `0x99D08`.
