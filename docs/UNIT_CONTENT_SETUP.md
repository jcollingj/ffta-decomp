# Unit content setup routines

## Recovered routines and evidence

`ApplyUnitContentProperty33` occupies ROM offsets `0xCA22C–0xCA268` (60
bytes), and `LoadUnitContentEquipment` occupies `0xCA268–0xCA2E8` (128
bytes, including its literal pool). The isolated probe used
`build/verified-339-content-ref.elf`, SHA-256
`1cb594bdd8655baecca519a6272d57e187b872bb57b40fbe01002189fafb0d29`. The
USA base ROM identity is SHA-1 `4ac05441f4de70a4ec3dd932116346c61b8783d9`.

The first routine calls `GetUnitProperty(unit, 33)`. For a nonzero result `p`,
it stores `0xE4` at byte offset `0x40 + p`. There is no range check in the
routine. Its caller must ensure this computed destination is writable within
the unit object; tests use `p = 0x13`, destination offset `0x53`, inside the
established `UnitFieldView` extent of `0xFC` bytes. It then reloads unit byte
7. If that byte is nonzero and its 8-bit subtraction by 68 is at most 1, the
routine ORs mask `0x0800` (bit 11) into the halfword at offset `0x28`, retaining
the other bits. The subtraction result is narrowed to eight bits before the
comparison.

The second routine copies ten bytes from source entry offset 8 into unit offset
`0x2A` through `gCopyRecordMemory`. Those bytes provide five halfword item
slots. Its initial grant flag is true. It checks bit 15 of the unit halfword at
`0x28`; when set, it clears the flag and skips the bit-13 predicate. Otherwise
it checks bit 13 and clears the flag if set. It always reads `ReadUnitField`
selector 3 after those checks. Only when the result is 23 does it call
`UnitField01Equals(unit, 94)`; a false result clears the grant flag. This read
and the conditional equality call still occur if a bit check already denied
the grant.

The routine iterates all five item slots, including zero IDs. When the grant
flag remains set, each iteration calls `AddInventoryItem(item, 1)`, reloads
that halfword and calls `AdjustInventoryEntryByte3(item, 1)`. It then reloads
the halfword again and always calls `ApplyItemAbilityFlags(unit, item)`. The
return from `AddInventoryItem` is ignored. The existing declarations are used
for the copy, adjust, and item-ability helpers; `AddInventoryItem` is declared
in this owned source with its observed `int (u16, u8)` signature because the
existing inventory header does not declare it.

## Host fixtures and limits

The focused host test compiles the production source as a separate translation
unit. One executable links the real `src/unit_fields.c` and checks the actual
bit predicates, selector-3 read, and equality-94 behavior. A second executable
uses controlled predicate doubles to assert the exact short-circuit and
call-order trace, including selector 3 and equality 94 after bit 15 denied a
grant. These are separate claims: the controlled trace proves ordering, while
the real-helper fixture proves predicate composition.

Both variants compare fixed whole live fixtures. Each `UnitFieldView` is
bracketed by a real sentinel word, and the 48-byte source entry has a trailing
sentinel; all guards, source bytes, and unit bytes participate in the expected
image comparison. The complete callback trace and inventory-visible fixture
are compared as well. Cases
cover zero and nonzero property results, byte-7 values 67, 68, 69, 70, and 255,
a getter mutation that changes byte 7 to 69 before the routine rereads it, both
grant-denying bits, selector 3 equal to 23 with field 01 equal to 94 or 93, and
a non-23 selector. Item fixtures include zero and repeated zero slots. A
controlled callback case changes slot zero from `0x1101` to `0x2202` during
`AddInventoryItem`, then to `0x3303` during the adjust callback; the trace
requires those reloaded values to reach the next two calls.

The target probe reports an exact 188-byte block match:

```sh
python3 tools/decomp/probe.py src/unit_content_setup.c \
  --start 0xca22c --end 0xca2e8 \
  --reference-elf build/verified-339-content-ref.elf --disassemble
python3 -m unittest tests.test_unit_content_setup_c -v
```

This documents the two routines' observed effects and call contracts; it does
not assign broader gameplay meaning to the numeric unit fields or establish a
maximum property value beyond the caller's writable-destination precondition.
