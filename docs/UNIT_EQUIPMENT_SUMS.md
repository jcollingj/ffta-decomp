# Unit equipment property sums

The source in `src/unit_equipment_value16.c` matches routines at `0x0CA394`
and `0x0CA3EC`. The second is an 18-byte forwarding wrapper that multiplies
the first routine's return by ten. Two zero bytes at `0x0CA3FE–0x0CA400` are
emitted by an alignment directive. This 108-byte block is integrated into the
matching build. `src/nonmatching/unit_equipment_sum.c` is a behavioral
candidate for the 48-byte routine at `0x0CA4F0`; its current probe is 44 bytes
and does not match, so it remains research source and earns no recovery
credit.

The field names in `UnitFieldView` remain structural. The recovered code reads
five halfwords at offsets `0x2A, 0x2C, 0x2E, 0x30, 0x32` and, for the first
reader, byte `0x3D`. It skips zero item IDs. The first routine gets property
30 from `GetUnitProperty`, narrows that value to a byte, increments it only
when byte `0x3D` equals exactly 1, and adds item property 16 values. It narrows
the final sum to signed byte before clamping negative results to zero. The
wrapper scales that return by ten. The separate property-sum candidate
narrows its u32 selector to a byte and accumulates the selected item property
values in u32 arithmetic.

The host harness compiles the matching reader and sum candidate as separate
translation units. It substitutes controlled doubles for the already
matching `GetUnitProperty` and `GetItemProperty` dependencies to check edge
values and callback-visible state. Fixtures include complete guarded unit
images and explicit expected call order; they do not establish gameplay
meaning for the selected properties.

The immutable probe reference is `build/verified-325-content-ref.elf`,
SHA-256 `c872e19d7191837986d66d74122e02a5eb92428ab73ff7cefb033639977035dc`.
The integrated source hash is
`5cf922ac973f663c2dd7d8993781a9c02c9200737ea9d7cb245e14ad2666a963`;
the separate sum candidate hash is
`5085f33ae47c1966fdb99223fddaf86caca2f74166f960da57ffda8269faab6e`.
The copied prefix ends at `0x0CA394`; its length from `0x0C9644` is
`0xD50`, computed as the end offset minus the start offset. Review caught and
corrected an initial hand calculation of `0x750` before the matching build.
Reproduction commands:

```sh
python3 tools/decomp/probe.py src/unit_equipment_value16.c --start 0xCA394 --end 0xCA400 --reference-elf build/verified-325-content-ref.elf
python3 tools/decomp/probe.py src/nonmatching/unit_equipment_sum.c --start 0xCA4F0 --end 0xCA520 --reference-elf build/verified-325-content-ref.elf
python3 -m unittest tests/test_unit_equipment_sums_c.py -v
```

Bounded sum-source trials against the same 48-byte span: unsigned `i < 5`
with direct `half18[9+i]` emitted 60 bytes; signed `i <= 4` with direct field
indexing also emitted 60; a named pointer to `half18[9]` indexed by signed `i`
emitted 44. The last form is retained as the current behavioral candidate.
These are all nonmatches and do not contribute recovery credit.
