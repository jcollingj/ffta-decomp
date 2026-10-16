# Tree insertion research

`InsertTreeNode` is a source candidate for ROM interval `[0xC7AB4, 0xC7B08)`.
The immutable reference ELF used for the isolated probe was
`build/verified-277-container-ref.elf`, SHA-256
`f4572a7c8e404ad75864cd1a1c757cae704bba53100e38b471695363a646fa22`.

## Original instruction evidence

Raw Thumb disassembly starts with `push {r4,r5,r6,lr}` at `0xC7AB4`. The
final instruction is `bx r1` at `0xC7B06`, ending exactly at `0xC7B08`.
There are 42 two-byte instruction units (84 bytes) in the interval; the final
pop/pop/bx return sequence is executable code, with no trailing padding.

The instructions establish the argument roles and behavior directly:

| Address | Original operation | Meaning established by the instruction |
|---|---|---|
| `C7AB4–C7ABC` | save r4-r6; copy r0/r1/r2 to r5/r4/r6 | owner, payload, key are retained |
| `C7ABC–C7AC2` | load `[r5]`; set 24; call `AllocatorAllocate` | allocator owner at owner+0; exact request 24 |
| `C7AC4–C7AD2` | store r4 at node+4, r6 at node+0, zero +8,+12,+16,+20 | payload/key and parent/children/flags initialization |
| `C7AD4–C7ADA` | load owner+8; compare null; assign new node to owner+8 if null | root is read after allocation and initialization |
| `C7ADE–C7AE6` | load current node word 0; compare key; `bge` to right branch | signed `key < node.word0` selects left; equality selects right |
| `C7AE8–C7AF0` | inspect current node+12; attach there if null | left child offset 12 |
| `C7AF2–C7AFC` | inspect current node+16; attach there if null | right child offset 16 |
| `C7AFE` | store parent at new node+8 | parent is assigned after a non-root attachment |
| `C7B00–C7B06` | copy new node to r0; return | returns the allocated node |

This is a signed comparison: the original `cmp r2,r0` followed by Thumb
`bge` selects the right branch for equality and for keys greater than the
stored word. The harness distinguishes both signed extremes, equality, root,
both immediate children, a deeper insertion, and a root mutation performed
inside the allocation dependency.

## Candidate and probe

The C candidate uses the recovered `TreeCountNode` layout and compatible
`AllocatorAllocate` declaration. The host allocator returns native-sized
retained storage while asserting the requested target size is 24. Focused
command: `python3 -m unittest tests.test_tree_insert_c -v` (pass).

Isolated probe:

```sh
python3 tools/decomp/probe.py src/nonmatching/tree_insert.c \
  --start 0xC7AB4 --end 0xC7B08 \
  --reference-elf build/verified-277-container-ref.elf --disassemble
```

Eight semantically valid natural-source hypotheses were compiled. Each passed
the revised harness with the candidate compiled as a separate translation
unit. The harness uses only the public header, checks one allocator call with
size 24 and the complete pre-call owner image, then compares complete owner,
entire arena, and replacement-root images after each operation. Expected node
images are built separately with sentinel padding and explicit field writes.
The test command is `python3 -m unittest tests.test_tree_insert_c -v`.

The best
candidate is v06: it emits an 84-byte symbol and differs in 42 bytes across the
84-byte symbol/block span. Every candidate passed the same host harness, which
checks the fixed signed minimum/maximum, equality-to-right behavior, empty root,
left/right/deep insertion, all initialized node fields, allocator request and
identity, whole owner/node images, exact returned node, and a root change
performed by the allocator callback. No C or compiler tricks were used.

| Variant | Natural source change from v01 | Symbol bytes | Symbol differences | Full-block differences |
|---|---|---:|---:|---:|
| v01 | baseline source | 80 | 46 | 50 |
| v02 | store key before payload | 80 | 46 | 50 |
| v03 | initialize parent, flags, then children | 80 | 48 | 52 |
| v06 | use `while (parent != 0)` for traversal | 84 | 42 | 42 |
| v07 | reverse operands: `parent->unknown00 > key` | 80 | 46 | 50 |
| v08 | test child non-null first, otherwise attach | 82 | 49 | 51 |
| v09 | explicit cast on returned node | 80 | 46 | 50 |
| v10 | express signed less-than as negated greater-or-equal | 80 | 46 | 50 |
| v11 | separate root/cursor and inner signed-key local | 80 | 44 | 48 |

Hashes, in table order: `72b095e37fb1ed9cee5465327b2c7ff18211f782d031b2f48349668fc32fd7a4`,
`f9eb07d3e5dac88d6e5354db36e8d4ce1431f9701d8cc121195d21eb2299e67f`,
`1fd8553b82b7068c04b1b6c2c822d9549b6466e16fc7fe814fb1c2118e66474d`,
`1a18498a65ae7f6d929ce544d2df3ab226a4105781ea8106c738d9706e5cf92a`,
`5c16ce398e25f34c9bb9467d295dcfb8b5144583a72b8047971fac2a166331d0`,
`bfab57015ef3ef51c193952c6c22ec23aeb7479ab496a8b7f014199efe79e4c1`,
`a4fbfb6437a4bafe660196dc407150667e0f8749b30a757157081324657d4787`, and
`82a0c8b7b51ab1edd4ed880172111d5a07fd14f127b58c1275fab7a0f8c3066b`.
The additional v11 source hash is
`ead431e92839a3a75344bd40e5ad7cf605171745b565c6d2c412db3586f0c7d0`.
The probe reports this research symbol as un-inventoried; no function or code
byte is claimed as recovered.

The exact variant construction used for the bounded trial set is preserved
below. Start from v01 as recorded by the hash above. v06 is the source kept in
`src/nonmatching/tree_insert.c`; its v01 precursor is reproduced by replacing
`while (parent != 0) {` with `for (;;) {`.

```python
# Reconstruct baseline from the retained v06 source and verify its hash.
from pathlib import Path
import hashlib
baseline = Path("src/nonmatching/tree_insert.c").read_text().replace(
    "while (parent != 0) {", "for (;;) {")
assert hashlib.sha256(baseline.encode()).hexdigest() == (
    "72b095e37fb1ed9cee5465327b2c7ff18211f782d031b2f48349668fc32fd7a4")

variants = {
    "v01": baseline,
    "v02": baseline.replace(
        "    node->unknown04 = payload;\n    node->unknown00 = (u32)key;",
        "    node->unknown00 = (u32)key;\n    node->unknown04 = payload;"),
    "v03": baseline.replace(
        "    node->parent = 0;\n    node->child_a = 0;\n"
        "    node->child_b = 0;\n    node->flags = 0;",
        "    node->parent = 0;\n    node->flags = 0;\n"
        "    node->child_a = 0;\n    node->child_b = 0;"),
    "v06": baseline.replace("for (;;) {", "while (parent != 0) {"),
    "v07": baseline.replace(
        "if (key < (int)parent->unknown00)",
        "if ((int)parent->unknown00 > key)"),
    "v08": baseline.replace("for (;;) {", "while (parent != 0) {").replace(
        "                if (parent->child_a == 0) {\n"
        "                    parent->child_a = node;\n                    break;\n"
        "                }\n                parent = parent->child_a;",
        "                if (parent->child_a != 0) {\n"
        "                    parent = parent->child_a;\n                } else {\n"
        "                    parent->child_a = node;\n                    break;\n"
        "                }"),
    "v09": baseline.replace(
        "    return node;",
        "    return (struct TreeCountNode *)node;"),
    "v10": baseline.replace(
        "if (key < (int)parent->unknown00)",
        "if (!(key >= (int)parent->unknown00))"),
}

# Additional v11 expression-lifetime trial. Each replacement is the exact
# source transform used to produce /tmp/v11_root_cursor_local_key.c.
v11 = baseline.replace(
    "    parent = owner->tree_root;\n"
    "    if (parent == 0) {\n"
    "        owner->tree_root = node;\n"
    "    } else {\n"
    "        for (;;) {\n"
    "            if (key < (int)parent->unknown00) {",
    "    {\n"
    "        struct TreeCountNode *root = owner->tree_root;\n"
    "        if (root == 0) {\n"
    "            owner->tree_root = node;\n"
    "        } else {\n"
    "            parent = root;\n"
    "            for (;;) {\n"
    "                int compare_key = key;\n"
    "                if (compare_key < (int)parent->unknown00) {"),
).replace(
    "        node->parent = parent;\n"
    "    }\n"
    "    return node;",
    "            node->parent = parent;\n"
    "        }\n"
    "    }\n"
    "    return node;"),
variants["v11"] = v11
```

The eight bounded variants plus v11 were compiled with `tools/decomp/probe.py` over the same ROM bounds
and immutable ELF. Two additional exploratory forms were excluded from the
eight: unsigned key ordering fails the signed-minimum fixture, and reading a
`u32` word through an `int *` relies on incompatible pointer aliasing.
The chosen v06 traversal's guard is reached only with a non-null current node
for the valid tree shapes under test; its attach branches exit immediately.
The additional v11 variation keeps a separate root local, copies it into the
cursor for the nonempty branch, and declares a signed key local inside the
traversal loop. It passed the same separate-translation-unit whole-image host
harness. Its symbol is 80 bytes with 44 symbol-aligned differences; the full
84-byte block differs at 48 byte positions, so v06 remains the best candidate.
