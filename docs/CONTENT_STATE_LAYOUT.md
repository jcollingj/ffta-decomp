# Content state RAM prefix

## Evidence

The original USA ROM used for inspection has SHA-1
`4ac05441f4de70a4ec3dd932116346c61b8783d9`, matching the expected image.
The following reads were verified by extracting the bounded bytes directly from
`baserom.gba` and disassembling them as Thumb code:

- `ReadCurrentContentIndex` at ROM `0xA018` loads the literal `0x020020D0`, adds
  `0xA0`, and performs `ldrh`; its read is the native halfword at RAM `+0xA0`.
- `ReadContentIndexOrFallback` at ROM `0xA024` loads the same literal, adds
  `0xA2`, and performs `ldrh`. If this halfword is zero, it adds `0xA0` and
  performs a second `ldrh`; otherwise it returns the `+0xA2` value.
- `ReadContentValue0` at ROM `0xC84F4` and `ReadContentValue1` at `0xC8530`
  call `GetGameFlag(0x253)`. On the set branch they load the literal
  `0x020020D0`, add `0xA6` or `0xA7`, and perform `ldrb`. On the clear branch
  they use the current index, scale by 40, load a pointer at record offset
  `+0x1C`, and read byte `+0` or `+1` through that pointer.

These observations establish accesses through RAM byte `+0xA7`. They do not
establish the extent of the containing RAM object or the meaning of bytes
`+0xA4` and `+0xA5`.

## Shared declaration

Use `struct ContentIndexState` from `include/content_state.h` as one shared
native-width type with a known prefix ending just after byte `+0xA7` (168
bytes total):

```c
struct ContentIndexState {
    u8 unknown00[0xa0];
    u16 currentIndex;   /* +0xa0 */
    u16 fallbackIndex;  /* +0xa2 */
    u8 unknownA4[2];    /* +0xa4..+0xa5 */
    u8 value0;          /* +0xa6 */
    u8 value1;          /* +0xa7 */
};
```

The fields use the game's existing fixed-width `u8` and `u16` types. The
compiler's native alignment places the halfwords at the observed even offsets.
The declaration describes only the proven prefix; no size or layout is claimed
for RAM beyond `+0xA7`. Native offset assertions belong in host harnesses, while
target-source assertions must follow the GCC 2.9-compatible patterns in
`config/luna_lessons.json` (L016).

The shared declaration is in `include/content_state.h`; matching content-index
and value readers and the research copy in `src/nonmatching/unit_lookup.c` use
it. `include/content_record.h` remains unchanged.

## Implementation and verification

`include/content_state.h` is used by `src/content_index.c`,
`src/content_values.c`, `src/nonmatching/unit_lookup.c`, and the authorized host
harnesses. The matching content readers retain their original field accesses;
`unit_lookup.c` remains a nonmatching research candidate. The existing
content-index harness now compiles `src/content_index.c` as a separate
translation unit and retains exhaustive checks across all 65,536 values for
each halfword domain. The duplicate state declaration in the content-consumer
harness was migrated under its explicit file lease.

The composition harness at `tests/c/content_state_harness.c`, launched by
`tests/test_content_state_c.py`, links `src/content_index.c` and
`src/content_values.c` as separate translation units. Its controlled
`GetGameFlag(0x253)` stub distinguishes flag-set RAM bytes from flag-clear
indexed native `const ContentRecord` pointers. Both paths compare the full
168-byte known prefix, record data and pointed-value guards, and
sentinel-initialized result storage against fixed expected images. The flag-set
case uses flag value 2 to distinguish any-nonzero behavior from equality to 1.
The flag-clear case sets current index 0 and fallback index 1, so selecting the
wrong index reader yields distinguishable record bytes.

The parent-provided immutable reference `build/verified-280-content-ref.elf`
(SHA-256 `314354ee73629bd0e68810afba1f2aca42764fe132abce852a2cf3f4c84d2a90`)
was used for isolated probes. `src/content_index.c` matched both symbols and all
40 original block bytes; `src/content_values.c` matched both symbols and all 124
original block bytes. The first parent review found that the original composition fixtures did not
distinguish current index from fallback index, and used flag value 1 only. The
harness was revised to oppose current 0 against fallback 1 and to use flag value
2, making both behaviors discriminating. Parent feedback: task packets should
name these branch-discriminator fixtures explicitly. The focused content-index,
content-state composition, and content-consumer harnesses passed. The nonmatching `unit_lookup.c`
translation unit also compiled with host warnings treated as errors.

The parent-provided immutable reference `build/verified-280-content-ref.elf`
(SHA-256 `314354ee73629bd0e68810afba1f2aca42764fe132abce852a2cf3f4c84d2a90`)
was used for isolated probes. `src/content_index.c` matched both symbols and all
40 original block bytes; `src/content_values.c` matched both symbols and all 124
original block bytes. Focused content-index, content-state composition, and
content-consumer harnesses passed. The nonmatching `unit_lookup.c` translation
unit also compiled with host warnings treated as errors.

No source credit follows from this representation refactor. Unknown RAM fields,
record-table extent, valid index ranges, and the semantic roles of these values
remain open.
