# Equipped-stat routines

The four adjacent Thumb routines at ROM offsets `0x0CA624–0x0CA744` were recovered as C in `src/unit_equipped_stats.c` and promoted into the matching build. They match the verified reference block byte for byte. The reference is `build/verified-325-content-ref.elf`, SHA-256 `c872e19d7191837986d66d74122e02a5eb92428ab73ff7cefb033639977035dc`. The four symbols are each 72 bytes:

| Export | ROM offset | Base halfword | Item-property selector |
| --- | ---: | ---: | ---: |
| `GetUnitEquippedStat20` | `0x0CA624` | `+0x20` | 10 |
| `GetUnitEquippedStat24` | `0x0CA66C` | `+0x24` | 12 |
| `GetUnitEquippedStat22` | `0x0CA6B4` | `+0x22` | 11 |
| `GetUnitEquippedStat26` | `0x0CA6FC` | `+0x26` | 13 |

The raw entry code shifts the flag left by 24 bits and compares the result with zero, so only a nonzero low byte enables equipment additions. It snapshots the selected halfword before callbacks. When enabled, it reads five halfwords from `+0x2A`, `+0x2C`, `+0x2E`, `+0x30`, and `+0x32` in order. A zero item skips the callback. Every nonzero item calls `GetItemProperty` at `0x0CA7A4` with the routine's selector. The next slot is read after the callback, so a callback may affect later iterations.

The equipment values accumulate as unsigned 32-bit values. The original then adds the low 16 bits of that sum to the signed base and narrows the result to signed 16 bits. It caps values above 999 and has no lower clamp. The C uses a `u32` accumulator and adds its signed-16 narrowing to the signed base. Both operands promote to `signed int`; their sum is within `[-65536, 65534]`, so this step has no signed overflow. The exported C functions return `signed int` to preserve the sign-extended result in `r0` for callers.

The field accesses use the existing `UnitFieldView` declaration: the four bases are `half18[4]`, `[6]`, `[5]`, and `[7]`; the five equipment values are `[9]` through `[13]`. No duplicate unit layout was added.

The matching linker places `src/unit_equipped_stats.c` at `0x0CA624`, after the recovered unit stat wrappers and copied gaps, and before `LoadContentUnitRecords` at `0x0CA744`. It asserts all four function starts and 72-byte intervals. The region ledger lists four matching C functions and one 288-byte decompiled region; the copied prefix and intervening gaps remain separate.

The host harness compiles the production C as a separate translation unit and provides a controlled `GetItemProperty` double. Fixed whole-`UnitFieldView` images and explicit call traces cover all four offsets/selectors, a zero flag, high bits with a zero low byte, nonzero low-byte flags, skipped zero item IDs, repeated IDs, callback mutation of a later equipment slot, a callback mutation of the already-snapshotted base, no equipped items with the flag enabled, signed-16 wrap, unsigned 32-bit accumulator wrap over two callback returns, the upper cap with and without equipment, and negative values without a lower clamp. It does not host-execute the recovered `GetItemProperty` implementation or claim meanings for these numeric selectors.

Verification performed:

- `python3 -m unittest tests.test_unit_equipped_stats_c -v` passed.
- `python3 tools/decomp/probe.py src/unit_equipped_stats.c --start 0xCA624 --end 0xCA744 --reference-elf build/verified-325-content-ref.elf --disassemble` reported `MATCH`, 288 candidate and original bytes, and zero differing bytes.
- The probe ELF reports each exported symbol at its assigned address with size `0x48`.
- Focused linked comparisons for all four manifest entries passed, including address and size checks. `make` confirmed the complete output ROM still matches `baserom.gba`.
