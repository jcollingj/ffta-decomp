# Content coordinate lookup wrappers

## Recovered behavior

The two wrappers occupy `[0x9961C, 0x9966C)` in the original USA ROM. The
80-byte block SHA-256 is
`1e2261fde3194957b73dfc8afbcce56adce02ce54ad09c2d1b1587827258b2ed`; the
ROM SHA-1 is `4ac05441f4de70a4ec3dd932116346c61b8783d9`. Disassembly of the
immutable reference ELF (`build/verified-295-content-ref.elf`, SHA-256
`51ae8c96d9c1c677a579c103cd67e2bc8769fbdd5e7a31064fb92372f5828c75`)
confirms both wrappers are 40 bytes:

| ROM interval | callback literal | behavior |
| --- | --- | --- |
| `0x9961C..0x99644` | `0x0809AB7D` (`ContentCoordinateSelectCallback`) | select by signed entry positions shifted right 5 |
| `0x99644..0x9966C` | `0x0809ABB1` (`ContentUnitCoordinateSelectCallback`) | select by unsigned unit coordinate bytes |

Each routine reserves 12 stack bytes, clears the selected pointer at `[sp+8]`,
stores argument `r1` and `r2` as bytes at `[sp+4]` and `[sp+5]`, loads the
list owner from context offset `+0x10`, and directly calls
`VisitLinkedNodes` at `0x080C7CC0`. It passes the corresponding callback literal
as the callback argument and the stack query as callback context. The return
value is the pointer at `[sp+8]`. An empty list or no match returns null; the
visitor stops after the first match. The callbacks do not read query `unknown00`
or `unknown06`; those stack bytes are uninitialized in the original and remain
opaque.

The position callback compares signed query bytes with signed entry shorts
`position08` and `position0C`, each shifted right 5. The unit callback compares
the signed query bytes with unsigned unit bytes at offsets `0xF6` and `0xF7`.
Thus a query of `-1` cannot match a unit coordinate byte `255`. Both wrappers
accept `int` inputs in the recovered C API because the observed ABI stores each
argument with `STRB`; inputs are narrowed to their low byte. The immediate
wrapper instructions establish this truncation. A whole-program inventory of
direct and indirect callers was not established, so the `int` declaration does
not claim an exhaustive caller audit.

The callback names in the source are distinct from each other. The literal
callback addresses above are not the wrapper's BL target: both wrappers call
the shared visitor. `ContentCoordinateQuery.unknown00` remains a numeric opaque
word; this wrapper does not initialize it, and no meaning is assigned to it.

## Verification

The host composition harness compiles both production callbacks, the real
`GetListOwnerWord0`, and the real `VisitLinkedNodes` as separate translation
units. Fixed whole-fixture comparisons cover context offsets `+0x10`, `+0x14`,
and `+0x18`; empty, absent, first-match, and duplicate-match cases; negative
query values against unsigned unit bytes; signed position shifts; and input
truncation. The harness does not inspect or assert values for the wrapper's
uninitialized query words.

Focused test:

```sh
python3 -m unittest tests.test_content_coordinate_lookup_c -v
```

The isolated agbcc probe used a temporary source copy with build-only Thumb
aliases for the two callback literals, since those callback symbols are not
named in the immutable reference ELF. Against that ELF, the candidate matched
all 80 bytes of `[0x9961C, 0x9966C)` exactly. The aliases were not added to
repository source or the matching build.
