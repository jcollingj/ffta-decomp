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
