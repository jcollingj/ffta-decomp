# Content-unit selection callback

## Original routine and caller contract

The callback occupies ROM offsets `0x9AB3C–0x9AB5C` (32 bytes). The immutable
286-function reference ELF is
`build/verified-286-content-ref.elf`, SHA-256
`463c56de5fa6e3ea3a512b3d319483ee254af625a38b42894083a384f3b917f2`.
The original ROM has SHA-1
`4ac05441f4de70a4ec3dd932116346c61b8783d9`. Hashing the exact 32-byte ROM
interval gives SHA-256
`9d9f6b2d8b299a27baf693fb8489787d459dab56fb03ac43da67309b1bcb7160`.

The argument type comes through an adapter. At `0x99544`, the wrapper passes
callback address `0x0809AB3D` and an output slot to `0x99840`. The wrapper calls
`VisitLinkedNodes` with the adapter at `0x9ABE8`. That adapter calls
`GetListOwnerWord0` on each list node to extract its native-width payload, then
uses the callback/context pair to invoke the payload callback. As a result,
`0x9AB3C` receives a pointer to a content entry, not a `ListCountNode`; the
callback returns that same entry pointer through its output context.

Direct Thumb disassembly of `[0x9AB3C, 0x9AB5C)` from `baserom.gba`:

| Address | Instruction | Effect |
| --- | --- | --- |
| `0x9AB3C` | `push {r4, r5, lr}` | Save registers. |
| `0x9AB3E` | `adds r4, r0, #0` | Preserve the original entry argument. |
| `0x9AB40` | `adds r5, r1, #0` | Preserve the output context. |
| `0x9AB42` | `ldr r0, [r4, #0]` | Load the entry's unit pointer. |
| `0x9AB44` | `bl 0xC8298` | Call `UnitField27HasBit12`. |
| `0x9AB48` | `lsls r0, r0, #24` | Test the helper's byte result. |
| `0x9AB4A` | `cmp r0, #0` | Select the match path. |
| `0x9AB4C` | `beq 0x9AB54` | A zero helper result skips the store. |
| `0x9AB4E` | `str r4, [r5, #0]` | Store the original content entry through context. |
| `0x9AB50` | `movs r0, #0` | Return zero for a match. |
| `0x9AB52` | `b 0x9AB56` | Skip the nonmatch return value. |
| `0x9AB54` | `movs r0, #1` | Return one for no match. |
| `0x9AB56–0x9AB5C` | `pop {r4, r5}; pop {r1}; bx r1` | Restore and return. |

The existing `ContentConsumerEntry` view now has a `UnitFieldView *object` at
offset zero. `UnitField27HasBit12` tests bit `0x1000` at unit offset `0x28`,
which is `UnitFieldView.half18[8]`; `ReadUnitField(unit, 27)` selects that same
halfword. A set bit stores the original entry through a
`struct ContentConsumerEntry **` context slot and returns zero. A clear bit
leaves the output unchanged and returns one. The callback dereferences the
entry at offset zero. The real helper handles a null unit pointer by returning
zero, so a null `entry->object` follows the nonmatch path.

## Bounds and host evidence

The callback reads a native unit pointer at content-entry offset zero. The
helper reads the 16-bit unit field at offset `0x28`; tests use a complete
`UnitFieldView`. The output slot is written only on a match. The focused host
test compiles the callback, `unit_fields.c`, and its harness as separate
translation units. Fixed values cover a set bit 12 with unrelated flags, a
clear bit 12 with bits 11, 13, and 15 set, and a null unit pointer. Assertions
check `ReadUnitField` selector 27, the helper result, callback result, output
sentinel, and full entry/unit/context images. No direct list traversal or full
visit-trace result is claimed here.

The matching `ConsumeContentRecord` consumer also reads this same unit flag
halfword. Its entry view and harness use `UnitFieldView.half18[8]` instead of a
duplicate partial object declaration. The isolated probe reports an exact
184-byte block match, including the 182-byte function and two padding bytes.
The selection callback probe reports an exact 32-byte match at `0x9AB3C`, and
the function is now in the matching-build inventory.

Focused verification passed:

```sh
python3 -m unittest tests.test_content_unit_select_c tests.test_content_consumer_c -v
python3 tools/decomp/probe.py src/content_unit_select.c \
  --start 0x9ab3c --end 0x9ab5c \
  --reference-elf build/verified-286-content-ref.elf --disassemble
python3 tools/decomp/probe.py src/content_consumer.c \
  --start 0x2ba40 --end 0x2baf8 \
  --reference-elf build/verified-286-content-ref.elf --disassemble
```

`ReadUnitField` has other legacy selectors that convert pointers to `u32`
(selectors 9, 28, 34, 39, 56, and 68). Those unrelated paths are not exercised
by these callback tests and are not claimed to preserve host pointers. The host
test disables the compiler's pointer-to-integer width warning while compiling
the full translation unit; only selector 27 and the bit-12 helper are used.
