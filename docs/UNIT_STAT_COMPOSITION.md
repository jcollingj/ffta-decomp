# Unit stat composition host suite

`tests/test_unit_stat_composition_c.py` compiles each production dependency as
its own host translation unit and links the objects to
`tests/c/unit_stat_composition_harness.c`. The suite composes the real
`GetItemProperty`, `GetJobProperty`, `GetUnitProperty`, equipped-stat readers,
dispatch/cap wrappers and `ReadUnitEquipmentValue16` implementations.

The suite also composes `ReadUnitEquipmentValue14` and
`ReadUnitEquipmentValue15` from `src/nonmatching/unit_clamped_values.c`. Those
two readers remain explicitly nonmatching research and receive no recovery
credit. This host execution is not evidence of GBA execution, compiler matching,
or battery-save behavior.

The harness defines externally linked immutable `const struct Item gItems[303]`
and `const struct Job gJobs[4]` fixtures using the established production
types. All item modifier values fit the declared signed-char representation.
Only `ReadSaveByte` is controlled: it reads configured bytes and records the
exact requested-index trace. No item, job, property reader, stat reader, or
wrapper is stubbed. Every unit image is a full `UnitFieldView`; selectors 30
and 32 read the proven job bytes 5 and 7. These scenarios avoid selector 0,
whose real unit-property path may read offset `0x100`.

After every production call, the harness compares the complete live guarded
unit image with a snapshot taken after the fixture's explicit setup. The
fixed expected vectors are:

| Scenario | Inputs and staged result | Expected output and save trace |
| --- | --- | --- |
| Negative clamp, save bonus, repeated item | Five slots `[51, 7, 51, 0, 0]`, stat20 base 10, save13=0 and save1=20. Item51 selector10 is `120+20=140`; item7 signed `-3` converts to `0xFFFFFFFD` and clamps to 255; repeated item51 contributes 140. Sum 535; narrow to signed16 535; add base 10. | `GetUnitEquippedStat20` and capped wrapper both return **545**. Save reads `[13, 1, 13, 13, 1]`. |
| Positive special-bonus overflow | Slot `[51,0,0,0,0]`, stat20 base 10, save13=0 and save1=200. Modifier120 plus bonus200 gives320, which clamps to255. | **265**, trace `[13,1]`. |
| Fifth slot only | `[0, 0, 0, 0, 7]`, stat20 base 10, save13=0. The fifth slot's `-3` becomes `0xFFFFFFFD`, clamps to 255; four prior slots are zero. | **265**, trace `[13]`. A four-slot loop would return 10. |
| Signed results and narrowing | `[1, 2, 0, 0, 0]`, stat22 base 0, save13=1. Item selector11 results are `0xFFFFFFF9` and `0xFFFFFFF6`; u32 sum wraps to `0xFFFFFFEF`, narrows through u16 to signed -17, then adds to base. | Equipped and capped readers both return **-17**; trace `[13, 13]`. |
| Cross-selector contributions and caps | `[302, 148, 286, 0, 0]`, bases 800, save13=0 and save bytes 2/3/4=20. Selector10: `5+6+7=18`; selector11: `140+255+3=398`; selector12: `255+135+4=394`; selector13: `1+2+130=133`. | Stat20 **818**, trace `[13,13,13]`; mask3 for 20/24 also returns **818** (stat24 would return999), proving bit0 priority, with the same trace; stat22 capped **999**, trace `[13,3,13,13]`; stat24 capped **999**, trace `[13,13,2,13]`; stat26 **933**, trace `[13,13,13,4]`. Zero slots cause no item/save calls. Mask-zero dispatch is also asserted. |
| Job inheritance and reader16 | Unit job/fallback bytes are 1/3. Job1 inherits job2; job2 p30/p32 are 11/13. Job3 inherit=255 uses fallback job1, which inherits job2, so the complete chain also resolves to 11/13. Reader16 status byte is exactly 1; five slots `[51,148,286,0,7]` contribute `5+6+7+0=18`. | `GetUnitProperty(30)=11`, `(32)=13`, `GetJobProperty(3,1,30)=11`; `ReadUnitEquipmentValue16` returns **30**, scaled reader **300**. No save-byte reads. |
| Research readers14/15, negative sum | `[1,2,0,0,0]`; selector14/15 values are `-7` and `-10`, summing as u32 to `0xFFFFFFEF`. Reader14 base is signedD0[1]=20: unsigned addition wraps to 3 before signed-short conversion and lower clamp. Reader15 inherited p32 base is 13: addition narrows to signed -4 and lower-clamps. | Research `ReadUnitEquipmentValue14` **3**; `ReadUnitEquipmentValue15` **0**. No save-byte reads. |
| Research readers14/15, five repeated positive values | `[51,51,51,51,51]`; selector14/15 modifier is 30 for each item. Sum 150. Reader14 base 40 gives 190; reader15 base 13 gives 163 then upper-clamps to 100. | Research reader14 **190**; reader15 **100**. No save-byte reads. |

The focused host suite can be run with:

```sh
python3 -m unittest tests.test_unit_stat_composition_c -v
```
