# Counted content unit loader

The routine at ROM offset `0xCA744` occupies 96 bytes through exclusive end
`0xCA7A4`. Its first argument points to a content record, its second argument
is the first destination `UnitRecord`, and its third argument is a signed
limit. It returns the number of destination iterations completed. Its
relationship to maps, missions, or other product concepts is unknown.

The original instructions load a halfword count at record offset `+0` and a
native pointer at `+4`. Each source entry advances by 48 bytes. The destination
occupancy byte is at `UnitRecord` offset `+4`, and destination records advance
by 264 bytes. A source counter is compared with the record count, while a
separate index is compared with the signed limit and returned. Both advance
once per callback. Before each callback, the routine scans forward from its
retained destination cursor until that occupancy byte is zero. It calls the
copied routine at `0xC9644` with destination, source entry, and zero; after the
call it advances its counters and reloads the record count. The destination
cursor remains at the selected record so a callback that leaves it unoccupied
causes the next iteration to reuse it.

`InitializeUnitFromContentEntry` is a typed diagnostic name for that copied
dependency only. It has no recovered-C credit. No destination bound check is
present in the original function, and callers must provide enough source and
destination storage for the count, limit, and callback occupancy behavior.

The shared `ContentRecord` layout now describes the observed count at `+0`,
source pointer at `+4` on target, values pointer at `+0x1c`, and eight trailing
opaque bytes. Pointers retain native width in host tests. The host source
pointer is naturally aligned at `+8`, the values pointer at `+0x28`, and the
trailing opaque bytes begin at `+0x30`; the host record stride is 56 bytes.
The 20-byte opaque gap retains that exact extent on both ABIs; the target
record remains 40 bytes. A source entry has an opaque, typed 48-byte
extent so pointer advancement expresses the observed record stride without
assigning meaning to its contents. The remaining fields stay opaque where no
access has established semantics.

The inventory caveat is that table bytes at ROM address `0x0854CD54` do not
establish that every record is valid: index 0 decodes to count 1546 and pointer
`0x060B070B`, while indices 1 and 2 decode to counts 8 and 9 with pointers
`0x0852A4D0` and `0x0852A650`. These observations do not prove table bounds or
the purpose of those records. Synthetic index-zero fixtures test the generic
pointer-taking routine without claiming an actual ROM table entry is valid.

The host harness compiles the production source as a separate translation
unit. It checks signed nonpositive limits, zero source count, source and
destination strides, reuse versus advancement after callback occupancy
changes, callback-driven count growth and shrinkage, return values, guards,
and fixed whole-fixture images. The controlled initializer stub is only a test
dependency; it does not model the copied routine's implementation.

The shared record is already used by matching content consumers. The loader
joins the matching build and function ledger through its separately reviewed
96-byte carve; its copied initializer remains a diagnostic dependency only.

Compiler-shape trials used natural C forms only. A single-counter `while` form
emitted 80 candidate bytes; a single-counter `do` form emitted 92. Separating
the source count and returned index in a `do` loop emitted 104. A `for` loop
with separate counters and a precomputed next-source pointer emitted 96 bytes
with 18 differing bytes. Moving the accepted-count increment into the body did
not change that result. Reversing the for-clause update order reduced the
symbol-aligned difference to 10 bytes; advancing typed `source++` before
`index++` reduced it to 7, and that same update order with the precomputed
source removed matched all 96 bytes. The final candidate source SHA-256 is
`6d3425446ff1612c845ba75a66496d0d918bcdffbba06383b95e571c38da98fc`.
Every alternative kept the callback-mutation harness passing. The initial
isolated probe reported byte equality while the function was still
`UNINVENTORIED`. The separately reviewed carve now has a matching function
and region entry. The current matching comparison and ROM hash pass; the
copied initializer remains a diagnostic dependency only.
