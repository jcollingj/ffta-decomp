# Content-unit coordinate callback

## Original callback and direct-list caller

The callback at ROM offset `0x9ABB0` has a 54-byte Thumb symbol followed by
two zero padding bytes through `0x9ABE8`. The original ROM SHA-1 is
`4ac05441f4de70a4ec3dd932116346c61b8783d9`. The SHA-256 of the complete
56-byte block `[0x9ABB0, 0x9ABE8)` is
`ca10a5f3ce63208da77c21ad039f6e0a0b427d86625cf5b74da7ef4e052b49bb`.

The wrapper at `0x99644` establishes the callback contract directly. It writes
the two query bytes at stack offsets `+4` and `+5`, clears the selected-entry
slot at `+8`, loads the list owner from its input at `+0x10`, places Thumb
callback address `0x0809ABB1` in `r1`, and calls `VisitLinkedNodes` at
`0xC7CC0`. It passes no payload adapter or callback pair. The preceding
wrapper at `0x9961C` has the same shape and passes the neighboring direct-list
callback at `0x9AB7D`. The `0x9ABE8` adapter belongs to the separate generic
payload callback path.

Disassembly of `[0x9ABB0, 0x9ABE8)` shows the callback receives the list node
in `r0` and query in `r1`. It calls `GetListOwnerWord0` at `0xC7B74`, treats
that native payload as a `ContentConsumerEntry *`, and loads the unit pointer
from entry offset zero. It compares signed query byte `+4` with unsigned unit
byte `+0xF6`; if equal, it compares signed query byte `+5` with unsigned unit
byte `+0xF7`. Both equalities store the entry pointer at query offset `+8` and
return zero. Either mismatch leaves the selected slot untouched and returns
one. The unit pointer is dereferenced without a null check.

The shared `ContentCoordinateQuery` supplies the query fields. Its
`unknown00` is retained as an opaque `u32`; no meaning is inferred. The shared
`ContentConsumerEntry.object` points to the established `UnitFieldView`, whose
`bytesF1[5]` and `[6]` are the accessed offsets `0xF6` and `0xF7`. The shared
`ListVisitCallback` signature matches the recovered routine. The implementation
is named `ContentUnitCoordinateSelectCallback` to distinguish it from session
77's separate `ContentCoordinateSelectCallback`.

## Tests and matching evidence

The host harness builds the candidate, real `GetListOwnerWord0`, and real
`VisitLinkedNodes` as separate translation units. Direct callback cases use a
fixed equality matrix for signed query values `-128`, `-1`, `0`, `1`, and `127`
against unsigned unit bytes `0`, `1`, `127`, `128`, and `255`, independently
on each axis. This explicitly checks that signed `-128` does not equal
unsigned `128`. Every case compares the complete owner, node array, entries,
units, and query against a separate expected image, including an untouched
selected-entry sentinel on mismatch. Real-visitor cases cover a first match,
an absent match, and duplicate matches, with whole-fixture comparison and
selected-entry identity.

Focused host verification passed:

```sh
python3 -m unittest tests.test_content_unit_coordinate_select_c -v
```

The isolated agbcc probe against
`build/verified-295-content-ref.elf` (SHA-256
`51ae8c96d9c1c677a579c103cd67e2bc8769fbdd5e7a31064fb92372f5828c75`) reports
an exact 56-byte block match at `0x9ABB0..0x9ABE8`, comprising the 54-byte
callback symbol and two padding bytes. The source now occupies that ROM range
in the matching build and is recorded in the region/function inventory.
