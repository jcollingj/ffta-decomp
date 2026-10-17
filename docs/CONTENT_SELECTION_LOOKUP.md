# Content consumer selection lookup

## Verified original behavior

The lookup callback occupies ROM `0x9AB5C–0x9AB7C` (32 bytes). It saves the
callback context, calls the payload getter at `0xC7B74` with the list node,
and compares the first word of the returned entry with context word 0. On a
match it stores the entry pointer at context offset 8 and returns zero. On a
nonmatch it leaves the output untouched and returns one. `0xC7B74` is a
two-instruction getter (`ldr r0, [r0]`; `bx lr`), matching the existing
`GetListOwnerWord0` implementation.

The wrapper occupies `0x99560–0x99580` (32 bytes). It stores its selector at
stack offset 0, initializes stack offset 8 to null, leaves offset 4
uninitialized, loads the list owner from `ListCountContext` offset `0x10`, and
calls the linked-node visitor at `0xC7CC0` with callback address `0x9AB5D`
(Thumb bit set). It returns the pointer in stack word 2. The wrapper does not
read the unused stack word at offset 4. The visitor reads `Container.list_head`
at `+4`, follows `ListCountNode.next` at `+8`, and stops when the callback
returns zero.

The immediate caller is `ConsumeContentRecord` at `0x2BA40`. Its call at
`0x2BABC` targets `0x99560`; it then reads a halfword at returned pointer
offset `0x5C`. This supports one shared `ContentConsumerEntry` representation:
the unit/object pointer at target offset 0, unknown bytes through target
offset `0x5B`, and the observed halfword at target offset `0x5C`. Native host
pointer width changes the host offset naturally; no byte-offset pointer casts
are needed.

The complete call chain is therefore:

```text
ConsumeContentRecord (0x2BA40)
  -> ResolveContentConsumerSelection (0x99560)
     -> VisitLinkedNodes (0xC7CC0)
        -> callback trampoline (0x814225C)
           -> object-equality callback (0x9AB5C)
              -> GetListOwnerWord0 (0xC7B74)
```

The disassembly was read directly from the preserved `baserom.gba` using
`arm-none-eabi-objdump -D -b binary -marm -Mforce-thumb` with ROM-base VMA
`0x08000000`. The wrapper's callback literal is distinct from its actual
visitor call target; the latter is `0xC7CC0`.

## Planned source and tests

The generic pointer-equality predicate and typed wrapper are in separate
production translation units. The host harness composes the real
`GetListOwnerWord0`, `VisitLinkedNodes`, predicate, and wrapper. It compares
distinct unit identities through their entry object pointers. Cases cover no
match, first, middle, last, duplicate matches (first match wins), a selector
that differs from all entries, null selector/null object equality, and an
empty owner list. The callback checks its return and preserves a prefilled
result sentinel on nonmatch; the wrapper returns null for no match and an empty
list.

Build fixed expected state separately from actual fixtures. Compare the full
owner, node array, entry and unit arrays, and callback-visible context. In
particular, a nonmatch must preserve the output slot; a match may change only
the result slot; the wrapper must initialize its result slot to null; and the
unused context word must remain unobserved. Compile production sources as
separate translation units and use compatible callback and output pointer
types throughout.

## Candidate and focused verification

Under the shared-file lease, `ContentConsumerEntry` now combines the object
pointer and observed `value5c` field, and `ConsumeContentRecord` receives that
same type from the selection wrapper. The unused lookup-context word is a
`u32` and is deliberately left uninitialized by the
wrapper; the callback does not inspect it. The fill API's `void **` migration
from the preceding session is preserved.

Using immutable reference `build/verified-289-content-ref.elf` (SHA-256
`98459ab4b3ce0a15ff606bd4a509410da6e06cf29d2f78c756907db7e23040fc`):

| Candidate | Original extent | Result |
| --- | ---: | --- |
| `ContentSelectionEqualsObject` | `0x9AB5C–0x9AB7C` (32 bytes) | exact block match |
| `ResolveContentConsumerSelection` | `0x99560–0x99580` (32 bytes) | exact block match |
| `ConsumeContentRecord` | `0x2BA40–0x2BAF8` (184-byte block) | 182-byte symbol and full block match |
| `ContentUnitSelectCallback` | `0x9AB3C–0x9AB5C` (32 bytes) | exact block match |

The wrapper probe used a scratch-only typed Thumb alias for the new predicate
at `0x0809AB5D`; the immutable reference ELF and shared external-symbol files
were not changed. Focused host tests passed for all 16 `test_content*.py` tests,
plus the list visitor, list accessor, and list payload composition tests. The
lookup harness compiles the production wrapper, predicate, real payload getter,
and real visitor as separate translation units. No build, linker, function
inventory, or region-ledger changes have been made here, so these matches
remain probe evidence rather than integrated recovered-code credit.
