# Unit stat wrappers

The recovered wrappers select between equipment-derived stat helpers using the
low byte of a mask. Bit 0 has priority and selects stat 20 or 22; otherwise
bit 1 selects stat 24 or 26. With neither bit set, the selector returns zero
without calling a helper. The uncapped selectors forward the low-byte flag.
Their capped variants pass flag 1 and cap signed results above 999. The four
single-stat wrappers also pass flag 1 and apply the same signed upper cap.
The wrappers preserve the full signed integer result from dependency calls;
negative values are preserved.

The recovered routines occupy two copied-ROM spans: `0x0CA400–0x0CA4F0` and
`0x0CA520–0x0CA580`. The 48-byte interval between them remains copied. The
matching build replaces those spans with `src/unit_stat_dispatch.c` and
`src/unit_stat_caps.c`, respectively.

Against immutable `build/verified-325-content-ref.elf` (SHA-256
`c872e19d7191837986d66d74122e02a5eb92428ab73ff7cefb033639977035dc`), isolated
probes matched all 240 and 96 bytes. The wrapper symbols are 56, 64, 56, and 64
bytes in ROM order; the single-stat symbols are each 24 bytes. Eight strict
function comparisons also pass in the linked build.

`tests/test_unit_stat_wrappers_c.py` compiles each production source as its own
translation unit and supplies controlled doubles for the four recovered
equipped-stat dependencies. It checks every mask byte and flag byte, priority,
call counts, forwarded arguments, signed return boundaries, and unchanged unit
plus adjacent sentinel storage. Dependency and wrapper interfaces use signed
integer returns; test doubles use representative values from the observed
signed 16-bit result range.

Focused host tests pass with the equipped-stat callee tests. `make` confirms the
rebuilt ROM SHA-1 matches the supplied base ROM. The region audit reports 337
matching C functions; this integration adds eight, while total engine function
and code-byte counts remain unknown.
