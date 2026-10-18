# Unit derived bytes

`src/unit_derived_bytes.c` reproduces the updater block at ROM
offset `0x0CA2E8–0x0CA394` (172 bytes, including two trailing zero bytes).
`src/unit_derived_byte_read.c` reproduces the signed-byte reader at
`0x0CA610–0x0CA624` (20 bytes). The reader ends with `bx r1` at `0x0CA622`;
`0x0CA624` begins `GetUnitEquippedStat20`, so the reader interval has no added
padding.

The updater reads unit properties 34, 35, and 31 in order and narrows each
result to a byte. It calls `UnitStateE9Bit0` next; a nonzero result changes the
first byte to 1 unless it is already 1. It then reads byte `0x3D` and applies
the observed cases: 2 increments the third byte, 3 adds two, 4 sets the second
byte to 5, 5 sets the second byte to 3 only when its current value is 1 or 2,
and 6 sets the first byte to 3. All other values leave these bytes unchanged.
After the switch it calls the copied `SumUnitEquipmentProperty` routine at
`0x0CA4F0` with selector 17, adds the result to the third byte with byte
wrapping, and writes the first, second, third, and complement-of-third bytes at
`0xFC–0xFF`. These are observed operations; no gameplay meaning is assigned to
the fields or selectors.

`RefreshAndReadUnitDerivedByte` calls the updater and then reloads byte `0xFE`
as a signed byte. The host tests use the complete 0x108-byte `UnitRecord`,
including its opaque 12-byte tail after `UnitFieldView`, and compare whole
guarded records against fixed expected images. Controlled boundary doubles
assert getter selectors and call order, sum selector, flag ordering, and a
mode-byte mutation during the flag call. A separate composition build uses the
real `GetUnitProperty` and `UnitStateE9Bit0` implementations with a controlled
job-property/sum boundary. The reader test starts with a distinct value at
`0xFE` and verifies the signed value after the updater rewrites it.

The probes use an alias-bearing copy of the immutable
`build/verified-339-content-ref.elf` (SHA-256
`1cb594bdd8655baecca519a6272d57e187b872bb57b40fbe01002189fafb0d29`). The
copy adds a typed Thumb alias for the candidate updater at `0x080CA2E9` and a
typed alias for the still-copied sum routine at `0x080CA4F1`; it does not alter
or replace the immutable reference. Both candidate intervals match all
original bytes in isolated probes. In the matching build, the updater symbol
is `0xAA` bytes; the remaining two bytes in its 172-byte region are its
alignment fill. The reader symbol is 20 bytes. The copied sum alias supports
the caller relocation and earns no recovery credit for the sum routine.

The initial natural source spelling used a disjunction for case 5 and direct
tail-field stores; its probe emitted 176 bytes for the 172-byte block. A
bounded natural-C trial used the byte-wrapped range predicate visible in the
original (`(u8)(second - 1) <= 1`) and a named pointer for the four tail stores.
That source reproduced the complete block. It uses no opcode assembly; the
only assembly directive is zero-filled alignment. It has no volatile
register-control locals.

Verification:

```sh
python3 -m unittest tests.test_unit_derived_bytes_c -v
mkdir -p build/probes/unit_derived_bytes
cp build/verified-339-content-ref.elf build/probes/unit_derived_bytes/typed-reference.elf
arm-none-eabi-objcopy --add-symbol=UpdateUnitDerivedBytes=.text:0xCA2E9,global,function --add-symbol=SumUnitEquipmentProperty=.text:0xCA4F1,global,function build/probes/unit_derived_bytes/typed-reference.elf
python3 tools/decomp/probe.py src/unit_derived_bytes.c --start 0xCA2E8 --end 0xCA394 --reference-elf build/probes/unit_derived_bytes/typed-reference.elf --disassemble
python3 tools/decomp/probe.py src/unit_derived_byte_read.c --start 0xCA610 --end 0xCA624 --reference-elf build/probes/unit_derived_bytes/typed-reference.elf --disassemble
make -j4
python3 tools/decomp/compare.py UpdateUnitDerivedBytes
python3 tools/decomp/compare.py RefreshAndReadUnitDerivedByte
```

The matching build reported the original ROM hash. Both focused comparisons
passed at their assigned addresses and symbol sizes; the two focused host
tests passed.
