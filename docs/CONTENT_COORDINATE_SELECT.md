# Content coordinate selection callback

`ContentCoordinateSelectCallback` recovers the 52-byte routine at ROM offset
`0x9AB7C–0x9ABB0`. The disassembly reads the native payload from the list node
with `GetListOwnerWord0` (`0xC7B74`), then reads signed halfwords from the
entry at `+8` and `+12`. Each halfword is shifted right by five with signed
arithmetic semantics and compared with signed bytes at context offsets `+4`
and `+5`. If both values match, the original entry pointer is stored at
context offset `+8` and the callback returns zero. Either mismatch returns one
without changing the output. No meaning is assigned to the two coordinates.

The callback has the exact `ListVisitCallback` signature and can be passed
directly to `VisitLinkedNodes`. `ContentCoordinateQuery` gives the observed
context a native-compatible declaration: a numeric unknown word at `+0`,
signed selectors at `+4` and `+5`, two unknown bytes, and the selected entry
pointer at `+8`. `ContentConsumerEntry` now exposes the signed positions while
retaining the opaque fields and the previously recovered `value5c` field.
Native pointer expansion accounts for host offsets; target offsets remain
`position08=8`, `position0C=12`, and `value5c=0x5C`.

The focused host harness compiles the callback, real `GetListOwnerWord0`, and
real `VisitLinkedNodes` as separate translation units. It compares complete
owner, node, entry, unit, and query images for both matches and nonmatches. The
fixed boundary vectors cover signed-halfword extremes and negative shift
boundaries on both axes, including values whose shifted results cannot fit in
the signed-byte selector. It also checks each mismatch axis independently,
first-match stopping through the real visitor, and an empty list.

Focused verification:

```sh
python3 -m unittest tests.test_content_coordinate_select_c -v
python3 tools/decomp/probe.py src/content_coordinate_select.c \
  --start 0x9ab7c --end 0x9abb0 \
  --reference-elf build/verified-295-content-ref.elf --disassemble
```

The host test passes. The isolated agbcc probe reports an exact 52-byte match
at `0x9AB7C–0x9ABB0`, using reference ELF SHA-256
`51ae8c96d9c1c677a579c103cd67e2bc8769fbdd5e7a31064fb92372f5828c75`. This is
probe evidence only; the function has not been added to the matching build or
region ledger. After refining the shared entry view, the prior
`ContentUnitSelectCallback`, `ContentSelectionEqualsObject`, and
`ResolveContentConsumerSelection` blocks also probe as exact matches. The
updated `ConsumeContentRecord` remains an exact 182-byte symbol and 184-byte
block match with the new unit-flag lookup dependency aliased in a disposable
copy of the reference ELF; the verified reference file itself remains
unchanged.
