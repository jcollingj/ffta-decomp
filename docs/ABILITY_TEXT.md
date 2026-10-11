# Ability name editing

The first experiment changes **First Aid** to **Field Aid** in a separate modified ROM. The editable specification is `mods/ability-name.json`; run `make mod-ability-name` to build it. This is a bounded ROM data patch, not decompiled game code.

## Locating the text

Ordinary ASCII searches for the names did not match. The [FFTA randomizer data map](https://github.com/TojiKitten/FFTA-randomizer/blob/master/src/main/ffta/FFTAData.ts) identifies an ability/UI string pointer table at ROM offset `0x5567F0` (767 entries) and ability records at `0x55187C` (346 records, 28 bytes each). Its [character lookup](https://github.com/TojiKitten/FFTA-randomizer/blob/master/src/main/ffta/utils/charLookup.json) describes the custom encoding. These are research references; the patcher is a local implementation restricted to verified alphabetic names.

The following were checked directly against the matching USA ROM:

| Ability ID | Record offset | String ID | String offset | Original name |
| --- | --- | --- | --- | --- |
| 76 | `0x5520CC` | 221 | `0x554C4C` | First Aid |
| 26 | `0x551B54` | 285 | `0x554FA9` | Thunder |
| 23 | `0x551B00` | 398 | `0x5555BE` | Fire |

The first little-endian 16-bit field in each record is the string ID. String pointers are little-endian 32-bit GBA ROM addresses; subtract `0x08000000` to get file offsets.

## Encoding and limits

First Aid uses two-byte letters: uppercase A begins at `80 B0`, lowercase a at `80 CA`, space is `40 73`, and a zero byte terminates the string. Fire and Thunder use a leading `01` with single-byte alphabetic codes one greater than the corresponding two-byte letter's second byte.

The patcher validates the entire input ROM hash, the ability/string relationship, and the exact expected original bytes. It requires identical encoded length and writes a separate output with a hash manifest. First Aid and Field Aid both occupy 19 bytes including the terminator, so no pointer relocation is needed. All uses of that string ID see the replacement. This tool is not a general dialogue editor or compression encoder.

## Runtime proof

Use a copy of a verified in-game battery save with the modified ROM. Cold-boot it, load the save, then open the soldier ability list and inspect Field Aid. Loading a baseline emulator state into modified code is not the acceptance test because the state can restore cached data. Record verified screenshots and saved-game identity in `WORK_LOG.md`.

Verified on 2026-10-11: the original ROM shows First Aid and the modified ROM shows Field Aid after separately cold-loading the same post-tutorial battery save. Evidence: `logs/shots/original_ability_info.png` and `logs/shots/modified_field_aid.png`. The protected save is `checkpoints/baselines/post-tutorial-2026-10-11/ffta.sav`; full acceptance details are in `WORK_LOG.md`.

## Full heal and Regen variant

`make mod-field-aid` applies `mods/field-aid.json` through `tools/mods/field_aid.py`, producing `build/mods/field-aid/ffta.gba` and its manifest. The original 28-byte record is guarded before editing. Effect slots at record offsets 12–15 change from `5B 26 01 01` to `59 26 C3 01`: full HP recovery, existing debuff removal, native Regen, none. The [community ability map](https://datacrystal.tcrf.net/wiki/Final_Fantasy_Tactics_Advance%3AAbilities) supplied the effect IDs; native Aura's use of `C3` was also verified in our ROM.

Only five ROM bytes change: three name bytes and two effect bytes. Range, targeting, cost, and animation remain as First Aid; this is a free self-targeted heal. All users of this ability, including enemies, receive the modification. The original help text does not mention Regen. This is a data edit, not a custom healing routine or a change to the game's Regen formula.

The modified ROM SHA-1 is `fc6c3db921295005f8e00493d903eab539cf6a0e`. A cold-loaded baseline save was used to enter Herb Picking and test the ability in battle. Field Aid restored 42/59 to 59/59 HP; Regen status and later passive recovery were observed. See `WORK_LOG.md` and `RECORDING.md` for evidence and replay artifacts.

## First matching C recovery

The matching build now compiles `GetAbilityProperty` from `src/ability.c` at `0x080CCD50`. Its 272-byte region, including its literal pool and switch table, exactly matches the base ROM. The record layout is declared in `include/ability.h` with a compile-time 28-byte size check. Uncertain meanings retain offset-based names.

The original selector API returns: 0 name ID; 1 byte 2; 2 byte 4; 3–8 bytes 5–10; 9 a pointer to the four effect bytes; 10 the flags word at offset 16; 11–31 individual bits 0–20 of that word; 32 the halfword at offset 20; 33 byte 11. Other selectors return zero. It performs no ability-ID bounds check. This is original behavior, not a safety-enhanced replacement.

`SelectUnitAbilityList` at `0x080CCE60` also matches from C (228 bytes). It selects a pointer from `0x0851BA84` using unit/slot fields and returns property-reader selectors 37/38 through optional byte outputs. Full names for those fields and ranges remain provisional. `GetUnitProperty` and `GetUnitNameStringId` were recovered along this call path; the underlying `GetJobProperty` routine is now also recovered as matching C (see below).


## Job-property inheritance and save-image access

`src/job_property.c` now replaces the full 1,204-byte routine at `0x080C8570`. The packed 52-byte record at `0x08521A14` is represented by `include/job.h`; most field names are selector numbers until callers establish their meanings. Selector 2 directly returns byte 5 (inheritance). Every other supported selector follows byte 5 first: 255 redirects to the caller's fallback record, another nonzero value redirects to that record index, and zero reads the requested field. Unsupported selectors and inheritance cycles loop indefinitely, as in the original. No bounds checks or behavior changes were added.

The packed layout includes unaligned 16-bit fields, nibbles, three-bit fields spanning bytes, and 12-bit fields. Selectors 34 and 35 read the nibbles at byte 43; selector 33 reads byte 44. Their switch-body order matters for matching code generation. A size assertion guards the 52-byte compiler layout, while the complete compiled routine (including all packed reads and its switch table) is byte compared.

`src/save_access.c` recovers byte access at EWRAM base + `0x2030` and the alternate-buffer bit-flag readers/writers at buffer + `0x1F70`. These are low-level in-memory accessors, not a recovered on-disk save format or migration implementation. The save-image tail uses the supported GNU zero-length-array extension to leave its unknown extent explicit. Cold-load/save compatibility testing remains necessary for intentional format changes.


## Editable item-linked ability sets

`data/item_ability_sets.s` now authors the 225 original 20-byte records at `0x08520080–0x08521214`. Each `item_ability_set` line contains a count, an unknown byte, and job/index pairs. Omitted trailing fields default to zero; the comment identifies the record index. For example, `item_ability_set 1, 0, 255, 7` encodes one special pair and eight unused zero pairs. The matching build checks counts 0–9, unsigned-byte fields, and exactly 225 records.

Item property 18 selects the record (item byte offset 29). The 376 adjacent 32-byte item records use indices through 224. The end boundary is also the base of a separate table: original code at ROM `0xD234E` loads the pointer at `0xD237C` and indexes `0x08521214` with a 16-byte stride. This supports the fixed original extent; unchecked reads outside it are not valid authoring space.

The recovered `ApplyItemAbilityFlags` compares each pair's job byte with unit offset 7. On equality, it marks that unit ability index available and synchronizes paired state. Otherwise, job 255 writes the special slot at unit offset `0x3D`. These indices refer to unit ability state, not automatically to global ability definition IDs. The data has byte bounds checks, but complete semantic validation against every job/list remains pending.

The source table reproduces the original 4,500 bytes without copying ROM data at build time. Tests also verify a bounded edit and invalid-input rejection. This makes existing equipment-linked records editable; adding new global abilities, expanding tables, and changing progression still require their remaining source and reference recovery. Keep intentional variants separate from the matching build and verify them in game.


## Editable item records

`data/items.s` authors the 376 original 32-byte records at `0x0851D180–0x08520080`, ending exactly at the item-linked ability table. The recovered property reader proves the stride and field accesses, and the recovered inventory partition distinguishes item IDs through 375. This is the fixed original table extent; the reader itself still has no index bounds check.

Each record has four eight-byte groups in order:

| Source macro | Fields, in argument order |
| --- | --- |
| `item_header` | Record ID (not emitted), name ID, properties 1/2 (unsigned halfwords), unknown bytes 6/7 |
| `item_properties` | Properties 3–8 (unsigned bytes), property 9 (unsigned halfword) |
| `item_modifiers` | Eight signed bytes, read through selectors 10–17 |
| `item_tail` | Unknown bytes 0x18–0x1C, ability-set index, unknown bytes 0x1E/0x1F |

Assembler guards enforce consecutive IDs, field-group order, exactly 376 records, numeric widths, and ability-set references 0–224. Signed modifiers accept -128 through 127; the existing property reader's save-dependent unsigned clamping remains unchanged. Unknown values are retained explicitly rather than assigned speculative gameplay names. String IDs and other property relationships still need semantic validation as their consumers are recovered.

Tests reproduce all 12,032 bytes, reject malformed edits/layouts, and verify that legal boundary edits affect only their selected bytes. This table is matching-build source: intentional changes must use a separate variant build and in-game acceptance checks. A dedicated source-based item variant target and expanded item IDs are not implemented yet.

## Other recovered content structures under investigation

`include/record70.h` describes a packed 70-byte record consumed by the matching
`GetRecord70Property` routine at ROM offset `0xCE4DC`. The table base is
`0x55AE4C`; its record count and gameplay identity are not established. Fields
retain selector numbers rather than guessed gameplay names. Some selectors
return pointers into a record; three delegate to the matching `DecodeRecordByte` routine. That routine preserves
zero and adds 375 to nonzero input bytes (yielding 376–630).
The format is not yet an independently editable, fully authored table.

`data/counted_record_ids.s` is an authored 300-entry halfword list at
`0x529EE4`, ending with `0xFFFF` at `0x52A13C`. The original consumer at
`0xCE8A8` counts flags indexed by each ID plus 767 and then performs an extra
flag/linked-record check. The list builds byte-identically and supports bounded
source edits, but the consumer remains nonmatching research C. Its assembly
macro rejects early sentinels and enforces the original total size. Passing
these serialization guards does not prove that an arbitrary new ID is a valid
content reference or safe game-flag index. List expansion and gameplay names
require further recovery.

## Job equipment eligibility

`src/job_item_eligibility.c` recovers the original job/item eligibility routine.
Job property 36 selects one of the 35 words in
`data/equipment_class_masks.s` (`0x51D0F4–0x51D180`); ordinary eligibility tests
bit `item property 3 - 1`. All 116 original job records in the observed job-table
span reference indices 0–34. Each mask can now be edited in source with fixed
size, index ordering, and unsigned-word validation. This does not expand item
categories or the table.

The routine also contains rules beyond the mask: category 22 is accepted only
when the byte-sized job property 1 equals 4, independently of the mask. For
categories 23/25, item IDs 287/311/316 additionally require that group value;
the ordinary mask must still allow them. Changing a mask alone does not bypass
these explicit exceptions. Full category/group gameplay names remain under
investigation. Host behavioral tests cover defined shift counts (categories
1–32); the recovered code preserves original unchecked inputs and adds no new
bounds behavior.

## Save-dependent item amount

`CalculateItemAmount` now identifies item offset 6 as a halfword (`half06` in
`include/item.h`). The existing source-data authoring still emits its two
little-endian bytes, preserving compatibility with the original table.
When item byte 12 has bit 3 set, the routine uses offset 8 of a 10-byte record
selected by save-image byte `0x21B4` as a reduction. The recovered `ItemAmountAdjustmentApplies` predicate can
add 10, wrapping to a byte. The routine performs signed integer division of
`item.property02 * (100 - reduction)` by 100, converts the result to unsigned,
and applies `half06` as an unsigned floor. Without that item flag, it returns
`property02` directly; either path raises zero to one. Reductions above 100 can
therefore yield large unsigned results. Recovery preserves this behavior;
editor validation will need to respect actual content constraints once the
10-byte table and complete adjustment records are recovered.

The adjustment selector returns one of five record bases for selectors 2–6
(`0x528A44`, `0x528AA4`, `0x528B04`, `0x528B64`, `0x528BC4`), and null for all
other byte values. The predicate returns false for null before reading the item.
Otherwise it tests bit `item property 3 - 1` in the record's first packed word.
`ItemAdjustmentView` now represents that word followed by 30 packed item
halfwords (64 recovered format bytes). The 96-byte spacing alone is not a
recovered schema for the remaining bytes. These records remain copied data.

`ReadItemAdjustmentEntry` uses one-based indices into those 30 halfwords; index
zero or an invalid selector returns zero. The original reader does not bound
other indices. `ItemInAdjustmentList` searches indices 0–30 inclusively, so
item zero always reports present, including for an invalid selector. It retains
the original initial lookup and repeated lookups during the search.
