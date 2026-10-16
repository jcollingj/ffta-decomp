# Generic content consumer at `0x0802BA40`

This research recovers one bounded generic content-record consumer. Its semantic role is unknown. It is not identified as a map loader, and no map relationship is inferred.

## Raw evidence and bounds

The source ROM is the expected USA image, SHA-1 `4ac05441f4de70a4ec3dd932116346c61b8783d9`. The recovered block is ROM file offset `[0x2BA40, 0x2BAF8)`, 184 bytes. The function body ends at `0x2BAF6` (182 bytes); the final two bytes are zero padding. SHA-1 values are `9c0f28454fa8ea39a15f0b154efae442796e690a` for the full block, `3002ed6d5d95b0587af4b7e592431977c1c85494` for the body, and `1489f923c4dca729178b3e3233458550d8dddf29` for the padding.

The direct disassembly is reproducible with:

```sh
arm-none-eabi-objdump -D -b binary -marm -Mforce-thumb \
  --adjust-vma=0x08000000 --start-address=0x0802BA40 \
  --stop-address=0x0802BB00 baserom.gba
```

Observed control and data flow:

1. Call `0x08096D7C` with argument 0 and retain its returned context. Test bit `0x10` in the halfword at argument-0 offset `+16`.
2. With bit `0x10` set, call `ReadCurrentContentIndex` (`0x0800A018`), multiply the halfword index by 40, and add the `gContentRecords` base (`0x0854CD54`). Call `0x08099D08` with the context and a stack array of 13 native pointers. Its signed result is compared with 12 using signed `BLE`; values greater than 12 become 13, while negative values remain negative and cause zero loop iterations.
3. Visit the first `count` stack pointers in order. Each points to an entry whose first word is another pointer. A null nested pointer is skipped and iteration continues. A nonnull nested pointer is read at halfword offset `+40`; bit 0 increments the result. Output 0 receives the resulting count as a halfword; output 1 receives byte `+32` of the indexed 40-byte record.
4. With bit `0x10` clear, call `0x08099560(context, argument1)`, read halfword `+92` from its returned pointer, add one, and store the halfword result in output 0. Then call `0x08099544(context)`, normalize its result to zero or one, and add it to the results of the real `CountContextListAt14` (`0x08099CC4`) and `CountContextListAt18` (`0x08099CD0`) wrappers. Store the sum as a halfword in output 1.

The 40-byte record stride and byte `+32` agree with the separately observed content-record access path. Other record fields and the record's semantic role remain unknown. The helper addresses are provisional names for copied routines. Integration adds typed Thumb aliases from the observed direct BL targets; these aliases do not recover or count the helper bodies.

## Source and verification

`src/content_consumer.c` uses native pointer layers for the helper-filled stack list and the shared `struct ContentRecord` declaration in `include/content_record.h`. That declaration preserves the observed pointer at `+0x1c` and the eight-byte tail beginning at target offset `+0x20`; the consumer reads `unknown20[0]`. On a 32-bit target, the record is 40 bytes. On the 64-bit host, native pointer alignment makes the fixture record 48 bytes, and the harness checks the host field offsets directly. `src/content_values.c` and `src/nonmatching/unit_lookup.c` now include the shared declaration rather than duplicating it. `include/content_consumer.h` also defines the alternate selection result with its observed halfword at `+0x5c`.

The reference for the isolated probe is a build-only extension. Its base `build/probes/list_traversal/reference.elf` has SHA-256 `d9980760e27aec9a38e32ba98f1faf9451d9a51df13502b4cde165437cc92fa7`; the derived reference has SHA-256 `25bdd82e63bae0abe0d33c32cab93777a317d0747899a00be3c859d22f0d3c9d`. `arm-none-eabi-objcopy --add-symbol` added typed Thumb symbols for the four otherwise unnamed helper call targets at `0x08096D7D`, `0x08099D09`, `0x08099561`, and `0x08099545`. The base snapshot remains unchanged. Known symbols `ReadCurrentContentIndex`, `gContentRecords`, `CountContextListAt14`, and `CountContextListAt18` already exist in the base ELF.

The successful command was:

```sh
python3 tools/decomp/probe.py src/content_consumer.c \
  --start 0x2ba40 --end 0x2baf8 \
  --reference-elf build/probes/content_consumer/reference.elf --disassemble
```

The isolated probe reports 184 candidate bytes versus 184 original bytes, zero differing block bytes, function symbol start `0x2BA40`, and symbol size 182 bytes. The earlier pre-integration report marked the symbol `UNINVENTORIED`; the function is now in `config/decomp_regions.json` and the matching linker layout. Probe source SHA-256: `08287f5c15c18a4b7668c98450bbb24de340338cfe5ceb91077f538e77654918`; candidate block SHA-256: `666f74b2b9cb4b99a4f9d60c3c1776683a86636d0e683561eaf342763eb05f33`.

Compiler-shape revisions were bounded and preserved here as reproducible source changes:

| Hypothesis | Source change | Probe result |
|---|---|---|
| 1 | Initial natural loop used a `u16` qualifying counter and added the boolean helper after the two list counts. | 188 candidate bytes; differed from the 184-byte block. |
| 2 | Keep the qualifying counter full-width until the output store; call and normalize the boolean helper before the two count wrappers, matching observed call order. | 184 candidate bytes; 40 differing block bytes, first at `0x2BAB2`. |
| 3 | Express boolean normalization as `(0u - result) | result`, then shift right 31. | 184 candidate bytes; the boolean sequence still had an extra move and used a different register order. |
| 4 | Save the helper result in a distinct `helper_result` local before assigning the normalized `has_selection` value. | Exact 184-byte block match. |

`tests/test_content_consumer_c.py` compiles the candidate and test harness as separate translation units. It links the real recovered content-index reader, both real context list-count wrappers, and `CountLinkedNodes`; stubs are limited to the still-unrecovered context, pointer-list fill, selection, and selection-presence helpers. Outputs start from nonzero sentinels, so omitted writes fail even for negative/zero counts and wrapped results. Fixed expected output values and full fixture snapshots check both context and selection objects, entry/object pointer images, list nodes and links; the read-only content-record table and content-index RAM image are independently snapshotted before and after each indexed read. Cases cover signed counts `-1`, `0`, `1`, `12`, `13`, and `14` (clamped to 13); null nested-pointer continuation; multiple qualifying and nonqualifying objects; the content byte at `+32`; both flag branches; list-count sums; and halfword increment wrap from `0xFFFF` to zero, and nonzero boolean normalization for a high-bit-only `0x80000000` helper result.

Focused verification passed:

```sh
python3 -m unittest tests.test_content_consumer_c -v
```

## Matching-build integration

The copied interval `[0xA040, 0x99CB8)` was split mechanically around the new 184-byte block. `asm/rom_random_prefix.s` covers `[0xA040, 0x2BA40)` (137,728 bytes); `src/content_consumer.c` and its two alignment bytes cover `[0x2BA40, 0x2BAF8)`; `asm/rom_content_consumer_suffix.s` covers `[0x2BAF8, 0x99CB8)` (451,008 bytes). The retired `asm/rom_random_tail.s` was removed from the filesystem and the build list. Linker assertions pin the prefix start, function start, and suffix start/block length.

`asm/external_symbols.s` supplies typed Thumb aliases at `0x08096D7D`, `0x08099D09`, `0x08099561`, and `0x08099545`, derived from the four direct BL call targets. These aliases label calls into still-copied routines; their bodies remain copied and uncounted.

Verification after integration:

- `python3 -m unittest tests.test_content_consumer_c -v` passed.
- Host `cc -fsyntax-only` checks passed for `src/content_values.c` and `src/nonmatching/unit_lookup.c` after adopting the shared record header.
- `python3 tools/decomp/compare.py ConsumeContentRecord --disassemble` reported a 182-byte exact function match at `0x2BA40`.
- `make -j4` rebuilt the ROM and reported `ROM matches baserom`.
- `make progress` passed all 279 inventoried function comparisons and reported 43,244 source/generated bytes and 16,733,972 copied bytes. Total function/code denominators remain unknown.

This evidence establishes local byte equality and host-side behavior for the represented fixtures. It does not identify the consumer's gameplay role or any map format.
