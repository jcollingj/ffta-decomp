# Ability definition prefix

`data/ability_prefix.s` is a lossless authored representation of ability
records **0 through 76 inclusive** at ROM offset `0x55187C`. The fixed prefix is
77 records × 28 bytes = 2,156 bytes; the next byte remains copied. This boundary
is deliberately narrower than the 346-record declaration and community count.
Neither establishes the extent of this authored prefix.

The boundary includes record 76, First Aid. The local USA ROM was checked before
extraction against SHA-1
`4ac05441f4de70a4ec3dd932116346c61b8783d9`. The First Aid ability-name test
independently identifies ID 76 at record offset `0x5520CC`; its text and effect
bytes were validated during the cold-boot Field Aid experiment. The matching
`GetAbilityProperty` at ROM `0xCCD50` indexes `gAbilities[ability_id]`; the
linker places that table at `0x0855187C`. Its selector cases establish the
exposed field reads, and `include/ability.h` specifies the 28-byte structure.
Selectors 3–8 expose bytes 5–10, selector 9 returns the effects address,
selector 10 returns the flags word, selectors 11–31 read its individual bits,
selector 32 reads the halfword at offset 20, and selector 33 reads power at
offset 11. Remaining bytes keep offset-based unknown names. No new gameplay
meaning is assigned here.

`tools/decomp/ability_data.py` checks the ROM size and USA SHA-1 before local
extraction. Its decoder and encoder accept exactly 77 records and preserve all
28 bytes. `effects` and `unknown_16` are exact-length hexadecimal byte strings;
all numeric fields reject booleans, floats, negatives, and overflow. The GNU-as
source uses readable field arguments, per-field width guards, consecutive-ID
checks, and exact start/end symbols for the 2,156-byte prefix. It assembles
without reading the ROM.

Run `python3 tools/decomp/ability_data.py baserom.gba data/ability_prefix.s` to
recreate the reviewed source from the verified USA image. Tests include an
independent fixed-byte field vector, synthetic round trips, neighboring-byte
preservation, malformed-input checks, assembler rejection cases, and a
ROM-dependent exact-prefix comparison that skips when the verified ROM is
unavailable. The matching linker places the prefix at `0x0855187C`, asserts its
`0x085520E8` end, and resumes copied bytes there. The `gAbilities` alias resolves
to the prefix start, and its C declaration has no inferred array bound. This does
not establish a full table extent. This is authored-data provenance, not
recovered C code.
