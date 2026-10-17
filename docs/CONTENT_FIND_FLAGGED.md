# Find content entry with unit flag 12

`FindContentEntryWithUnitFlag12` recovers the original routine at ROM offset
`0x99544`. It visits the list owner at context offset `+0x10`, selects the
first payload entry whose nested unit has field 27 bit 12 set, and returns that
same entry pointer. It returns null when no entry matches. The function's
gameplay role remains unknown.

## Original wrapper evidence

The source ROM SHA-1 is `4ac05441f4de70a4ec3dd932116346c61b8783d9`. The full
28-byte interval `[0x99544,0x99560)` has SHA-256
`52885cefa4dae85fe40546a8a51013b80943a090610fce3154f496206cc2579a` and raw
bytes:

```text
00 b5 81 b0 00 21 00 91 03 49 6a 46 00 f0 76 f9
00 98 01 b0 02 bc 08 47 3d ab 09 08
```

The direct BL at `0x99550` targets `0x99840`. The word literal at `0x9955C`
is `0x0809AB3D`, the Thumb callback address for `ContentUnitSelectCallback`.
The wrapper zero-initializes a local output pointer, passes its address to
`0x99840`, reloads the selected pointer after traversal, and returns it. The
callback literal and the wrapper's BL target are distinct.

At `0x99840`, the code reads the list owner at context `+0x10`, passes the
payload adapter literal `0x0809ABE9`, and calls `0x080C7CC0`. The adapter at
`0x9ABE8` calls `GetListOwnerWord0` for each node and invokes the callback with
the extracted payload and output-pointer context. The callback at `0x9AB3C`
receives a `ContentConsumerEntry *`; when the nested unit's bit 12 is set it
stores that same pointer and returns zero to stop at the first match.

## Implementation and host checks

The wrapper uses the existing generic payload callback type end to end and
keeps its result as `struct ContentConsumerEntry *`. Its local result starts
at null. The consumer checks the returned native-width pointer for nonzero;
the matching expression uses the established `ListPayload` type, which is
pointer-width on the supported ILP32 target and LP64 host. It does not narrow
a pointer to `u32`.

`tests/test_content_find_flagged_c.py` compiles the wrapper, context visitor,
payload adapter, list accessor, list visitor, unit selector and unit-field
helpers as separate translation units. Fixed fixtures cover empty and
no-match lists, matches at the first, middle and last positions, duplicate
matches returning the first entry, null nested units, adjacent flag bits, and
matches that exist only in context `+0x14` or `+0x18`. Tests check native
pointer identity and compare full context, owner, node, entry and unit images
before and after each read-only call.

The existing consumer now uses the pointer-returning wrapper. Its stub returns
either an actual entry pointer or null, and fixed result cases check both
nonzero and null behavior while preserving its full fixture snapshots.
On LP64, an additional consumer fixture returns the opaque integer-derived
pointer value `1UL << 32`; the consumer never dereferences it. Its low 32 bits
are zero, so this case fails if the returned pointer is narrowed before the
nonzero calculation. The ordinary entry-pointer and null cases remain in place.

A scratch consumer mutant that narrowed the folded pointer result to `u32` and
shifted by 31 compiled, then failed the targeted LP64 fixture's full-fixture
assertion. The mutant and harness were temporary files; production sources
were not changed by this check.

## Byte verification

The wrapper candidate matches all 28 original block bytes in the isolated
probe. The updated `ConsumeContentRecord` candidate matches its 182-byte
function and the complete 184-byte block, including the two zero padding
bytes. The dependency probe used a writable build-only extension of
`build/verified-295-content-ref.elf` (SHA-256
`51ae8c96d9c1c677a579c103cd67e2bc8769fbdd5e7a31064fb92372f5828c75`) with a
Thumb function alias for `FindContentEntryWithUnitFlag12` at `0x08099545`; the
verified base ELF is not modified.

The retained matching consumer source is `src/content_consumer.c`, SHA-256
`2515cd54f954dc550c3a6e7f3525281a16cd2d96d887c67183ea8dfeb6cc1d5d`. It uses
the established `ListPayload` type, whose header asserts pointer-width equality
for the supported ILP32 and LP64 ABIs. The matching source folds the full
native-width integer representation and shifts by its top-bit position. This
is the retained byte-matching shape; it is not claimed to be the only source
shape capable of matching.

One direct-comparison alternative is preserved in
`docs/research/consumer_direct_pointer_compare.c`, SHA-256
`ca5105c1aa6fd4948ed580a877bf498001b937a04231fe629ac2897d07ea7457`. It uses
`has_selection = FindContentEntryWithUnitFlag12(context) != 0;` and produces
41 symbol-aligned differing bytes. This is one measured alternative, not proof
that all direct comparisons differ. The exact commands below reproduce both
probes; the derived dependency ELF was made from the immutable reference as
shown.

```sh
cp build/verified-295-content-ref.elf build/probes/content_consumer/find_flagged_reference.base.elf
arm-none-eabi-objcopy \
  --add-symbol FindContentEntryWithUnitFlag12=0x08099545,global,function \
  build/probes/content_consumer/find_flagged_reference.base.elf \
  build/probes/content_consumer/find_flagged_reference.elf
python3 tools/decomp/probe.py src/content_consumer.c \
  --start 0x2ba40 --end 0x2baf8 \
  --reference-elf build/probes/content_consumer/find_flagged_reference.elf --disassemble
python3 tools/decomp/probe.py docs/research/consumer_direct_pointer_compare.c \
  --start 0x2ba40 --end 0x2baf8 \
  --reference-elf build/probes/content_consumer/find_flagged_reference.elf --disassemble
```

Focused checks passed:

```sh
python3 -m unittest tests.test_content_find_flagged_c tests.test_content_consumer_c -v
```

The isolated probes reported an exact 28-byte wrapper block and an exact
184-byte consumer block. This verifies represented host cases and unchanged
local ROM bytes; it does not identify a gameplay role.

## Matching-build integration

The function now lives in `src/content_find_flagged.c` and has both a matching
function entry and a `decompiled_c` region entry. The earlier copied prefix was
replaced by the matching 96-byte `LoadAndPrepareContentUnitPool` function at
`[0x96E18,0x96E78)`, followed by the retained copied suffix `[0x96E78,0x99544)`
in `asm/rom_ctxvisit_suffix.s`. `Makefile` places those objects before the
existing content-entry wrappers. Linker assertions pin the suffix boundary,
the wrapper at `0x08099544`, and its 28-byte interval ending at
`ResolveContentConsumerSelection`.
The provisional `HasContentConsumerSelection` alias was removed.

Integration verification passed `make compare`, `make check-wrappers`, focused
consumer/wrapper tests, and direct `compare.py` checks for both the new 28-byte
function and unchanged 182-byte consumer function.
