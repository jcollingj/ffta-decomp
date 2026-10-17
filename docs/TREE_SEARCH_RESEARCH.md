# Tree search wrapper recovery

## Original block and instruction map

The bounded ROM interval is `[0xC7C14, 0xC7C40)`, 44 bytes. Thumb disassembly of the original bytes gives this control flow:

| ROM address | Bytes | Instruction / role |
| --- | --- | --- |
| `C7C14` | `00 B5` | Push LR. |
| `C7C16` | `82 B0` | Reserve 8 stack bytes for the two-word local context. |
| `C7C18` | `02 1C` | Copy owner argument from `r0` to `r2`. |
| `C7C1A` | `90 68` | Load `owner->tree_root` at target offset 8. |
| `C7C1C` | `00 28` | Compare root with null. |
| `C7C1E` | `0B D0` | Null branch to `C7C38`. |
| `C7C20` | `00 91` | Save key argument `r1` at local context offset 0. |
| `C7C22` | `00 20` | Set `r0` to zero. |
| `C7C24` | `01 90` | Initialize local result at context offset 4. |
| `C7C26` | `03 49` | Load the Thumb callback pointer from literal `C7C34`. |
| `C7C28` | `10 1C` | Restore owner argument to `r0`. |
| `C7C2A` | `6A 46` | Pass the local context address in `r2`. |
| `C7C2C` | `00 F0 74 F8` | Call `TraverseTreeInOrder` at `C7D18`. |
| `C7C30` | `01 98` | Load local result from stack offset 4. |
| `C7C32` | `02 E0` | Branch to common return at `C7C3A`. |
| `C7C34` | `39 7E 0C 08` | Embedded pointer literal `0x080C7E39`, Thumb entry `ListSearchCallback` at `C7E38`. |
| `C7C38` | `00 20` | Null-root result: set `r0` to zero. |
| `C7C3A` | `02 B0` | Release 8 local bytes. |
| `C7C3C` | `02 BC` | Restore return address. |
| `C7C3E` | `08 47` | Return. |

Thus the wrapper returns null for an empty tree. Otherwise it traverses in the existing in-order order, with a local `{key, result}` context initialized to `{key, NULL}`, and returns the callback's first match. Original bytes are independently visible with `xxd -g1 -s 0xC7C14 -l 44 baserom.gba`; the table above follows `arm-none-eabi-objdump -D -b binary -m arm -M force-thumb --start-address=0xC7C14 --stop-address=0xC7C40 baserom.gba`.

## Typed callback and host composition

`TreeSearchContext` contains a native-width `ListPayload key` and a
`TreeCountNode *result`. The target context remains 8 bytes; the LP64 host
context is 16 bytes, with the result at offset 8. `ListSearchCallback` has the
exact traversal callback type, `int (void *, void *)`. It reads the tree node
through the typed `CopyListOwnerWords` API: the numeric first word remains
`u32`, while the second word and search query use `ListPayload`. The callback
stores the matching node as a native typed pointer. The callback's target
block remains 48 bytes and its bytes match the original exactly.

The host integration harness links the actual wrapper, callback, in-order
traversal, and typed owner-word accessor. It checks empty and nonempty trees,
absent keys, duplicate payloads in different in-order positions,
stop-at-first-match behavior, distinct LP64 payload values with equal low 32
bits, and complete owner/node state. The focused callback harness also keeps a
same-base alias case using a union of the native node and context views. It
builds the expected whole-union byte image by copying a native pointer at
`offsetof(TreeSearchContext, result)`, then compares all storage. On target
this result store overlays the node's second word; on LP64 it overlays the
native-width second word at offset 8. This checks read-before-write ordering
for each layout. Cross-width aliases between a target `u32` output and
`ListPayload` output are not modeled with incompatible host lvalues; the
unchanged target instruction sequence establishes their target-width store
order.

The wrapper candidate compiles to a 44-byte symbol at `0x080C7C14`; its full
`[C7C14,C7C40)` block matches all 44 original bytes, including the callback
literal and return sequence. The tree payload migration was checked against
the immutable `build/verified-300-content-ref.elf`, SHA-256
`b509bd395e699edb5597f9ce97befea53b2d941efdf41a027995daeaffc88751`.
