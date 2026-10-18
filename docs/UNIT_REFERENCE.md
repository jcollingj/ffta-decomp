# Unit reference resolvers

Two adjacent routines are recovered in `src/unit_reference.c`:

| Symbol | ROM interval (exclusive end) | Symbol/region bytes |
| --- | --- | ---: |
| ResolveUnitReference | 0xCA1BC–0xCA1F0 | 52 |
| ResolveEncodedUnitReference | 0xCA1F0–0xCA22C | 60 |

The intervals include each function's literal pool and internal alignment.
Both complete intervals match the original USA ROM. Data tables and the
fallback buffer remain external dependencies; this recovery gives them no
authored-data credit and does not establish their full formats or bounds.

`ResolveUnitReference` reads the numeric word at unit offset zero and the
byte at offset 0x106. Mode zero indexes the pointer table at 0x085680DC;
mode one indexes the pointer table at 0x085516D0. Other byte values return
the buffer at 0x02001F1C without indexing a table. The numeric index is not
masked or narrowed, and original code performs no bounds check. The existing
0x108-byte `UnitRecord` provides the complete accessed extent.

`ResolveEncodedUnitReference` reads an unaligned little-endian 32-bit index.
The original loads four individual bytes at 0xCA1F2–0xCA204; a genuinely
packed four-byte structure reproduces those accesses. Values at most 0x7FFF
index table zero unchanged. Every larger unsigned value selects table one,
using the low 15 bits as its index. This is not merely a bit-15 test:
0x10000 and 0x80000000 both select table one at index zero. High-bit behavior
is preserved even though the complete set of game-produced inputs is unknown.

The two tables are represented as arrays of native pointers to opaque bytes,
so host tests preserve complete pointer identity. The pointee encoding and
gameplay meaning are deliberately not inferred from addresses alone. The
writable fallback buffer has an external byte-array declaration, returned
through the same read-only pointer contract.

## Evidence and reproduction

The immutable reference was `build/verified-343-content-ref.elf`, SHA-256
`ca1c1b7dd558ea7aa3fa44a8c8856cc44a76305e142218e417a06341bfca1044`.
A private copy added only the three data addresses needed for relocation.
Initial source emitted 112 bytes but differed in the encoded resolver's branch
layout. Expressing the original unsigned greater-than branch with explicit
if/else reproduced all 112 bytes. No register/volatile hints or opcode assembly
were used; the source's only assembly directive supplies zero alignment.

```sh
arm-none-eabi-objcopy --add-symbol gUnitReferenceTable0=0x085680DC,global --add-symbol gUnitReferenceTable1=0x085516D0,global --add-symbol gUnitReferenceFallback=0x02001F1C,global build/verified-343-content-ref.elf build/parent-343-unit-reference.elf
python3 tools/decomp/probe.py src/unit_reference.c --start 0xca1bc --end 0xca22c --reference-elf build/parent-343-unit-reference.elf
python3 -m unittest tests.test_unit_reference_c -v
make -j4 check progress
```

The host harness compiles production source as a separate translation unit.
It checks every mode byte, null table entries, a full-width ordinary index,
encoded values around 0x7FFF/0x8000 and larger high-bit values, and an actually
unaligned packed input. Fixed pointer expectations and complete live guarded
unit/input images are checked after each call. Sparse synthetic tables extend
to index 65536 solely to distinguish truncation; those sizes make no claim
about the original ROM tables. Protected game saves and emulator state are
not involved.

The carve planner/patch renderer generated the copied-prefix replacement and
object ordering. The ledger and linker separately record both functions and
the 112-byte region. The remaining prefix ends at 0xCA1BC, calculated as
0xC9644 + 0xB78. The next recovered routine remains at 0xCA22C.

## Reverse lookup and explicit pointer tables

`EncodeUnitReferencePointer` at 0xCA150–0xCA1BC is now recovered as 108 matching
C bytes, including literal pools. It takes an actually packed pointer, searches
table zero indices 0 through 724 first, then table one indices 0 through 106.
It returns the first matching index, ORing 0x8000 for a table-one match; a
missing pointer returns zero. Zero is therefore ambiguous with a successful
match at table-zero index zero. No alternative error behavior was introduced.
The packed pointer stays native-width in host fixtures; its target width is
four bytes. Its format is distinct from the numeric packed index.

The initial natural C implementation matched all 108 bytes using the immutable
345-function reference. Separate-TU tests cover first/last entries of both
searches, duplicates within and across tables, null entries, missing pointers,
and sentinel values just beyond each search limit. Valid matches compose with
the real encoded resolver. Every operation compares the complete live guarded
input image. The captured spans are consumer bounds, not a claim that all
possible callers are bounds checked.

The pointer definitions are now explicit authored data:

| Source | ROM start | Pointer count | Bytes | Exclusive end |
| --- | --- | ---: | ---: | --- |
| data/unit_reference_table0.s | 0x5680DC | 725 | 2,900 | 0x568C30 |
| data/unit_reference_table1.s | 0x5516D0 | 107 | 428 | 0x55187C |

Every observed pointer lies inside the 16 MiB ROM window. The assembler macro
accepts ROM-window addresses and rejects missing/extra entries. These are
explicit authoring constraints; they do not assert that the engine could never
consume a RAM pointer. The table symbols replace their former absolute linker
aliases. Original pointee bytes remain copied and are not counted as recovered.
These 3,328 bytes are data recovery, separately reported from matching C.

The generalized `tools/decomp/carve_plan.py` and `carve_patch.py` accept either
`src/NAME.c` or `data/NAME.s`. Both keep ledger edits and recovery admission
manual/reviewed, preserve unrelated objects, and omit empty remainder wrappers.
An attempted manual data carve exposed a dangerous assembler detail: a zero
`.incbin` count warned but included the rest of the input file. The failed build
was never published; the empty wrapper was removed, and planner/patch fixtures
now cover data carves ending exactly at an existing region boundary.

```sh
python3 tools/decomp/probe.py src/unit_reference_encode.c --start 0xca150 --end 0xca1bc --reference-elf build/verified-345-content-ref.elf
python3 -m unittest tests.test_unit_reference_encode_c tests.test_unit_reference_data -v
```
