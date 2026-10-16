# Adjacent content global-pointer getters

The bounded USA ROM block is `[0x96D7C, 0x96D94)` (24 bytes), SHA-1
`5a12b758e6545e47fe00c7030f7dc5bba6eba099`, and SHA-256
`c4f97810d7eaf97d33ee8b1e176db9dc0c255cfa748ba13c5da43c5714beb237`. The
complete USA ROM SHA-1 is `4ac05441f4de70a4ec3dd932116346c61b8783d9`.

At `0x96D7C`, Thumb code loads the pointer stored at RAM `0x0200F4B0` and
returns it. The incoming `r0` value is not used. The existing copied alias
`GetContentConsumerContext` and its signature are retained; the parameter name
is documented but does not imply an observed relationship to the global.
Bytes `0x96D82..0x96D83` are zero alignment before the pointer literal at
`0x96D84`; execution returns before reaching them, so they are alignment
bytes in the block rather than part of the routine's executed path.

At `0x96D88`, Thumb code loads the pointer stored at RAM `0x0200F4A8`, then
loads and returns the native pointer at owner target offset `+4`. The pointee
type and the owner's meaning are unknown, so its C view and return type use
`void *`. The member naturally appears at host
`offsetof` determined by host alignment (typically 8), while the GBA target
pointer places it at offset 4. The getter requires a valid owner pointer, as
the original dereferences it.

The production source is `src/content_globals.c`; its two target symbols each
occupy 12 bytes. Their source order matches the original block:
the first has 6 executed bytes, 2 zero alignment bytes, and a 4-byte literal;
the second has 8 executed bytes and a 4-byte literal. The complete candidate
block is 24 bytes and exactly matches the original (zero differing bytes).
The source SHA-256 is
`c9e064573ac83290b0ec1a8106c1c5258acf3cfd78dc701aa32f8741b6bec8f3`.

The isolated probe uses a writable derived copy of the verified immutable
reference ELF. These commands reproduce the RAM aliases without touching
shared symbol files:

```sh
mkdir -p build/probes/content_globals
cp build/verified-280-content-ref.elf build/probes/content_globals/reference-base.elf
arm-none-eabi-objcopy \
  --add-symbol gContentConsumerContext=0x0200F4B0,global,object \
  --add-symbol gContentGlobalPointerOwner=0x0200F4A8,global,object \
  build/probes/content_globals/reference-base.elf \
  build/probes/content_globals/reference.elf
python3 tools/decomp/probe.py src/content_globals.c \
  --start 0x96d7c --end 0x96d94 \
  --reference-elf build/probes/content_globals/reference.elf --disassemble
```

The immutable source reference has SHA-256
`314354ee73629bd0e68810afba1f2aca42764fe132abce852a2cf3f4c84d2a90`; the
derived alias reference has SHA-256
`e1332080de3df61c2964ff03df5727285386196651a91c8014dc6bfb82dbb884`.
`python3 -m unittest tests.test_content_globals_c -v` passes. The separate-TU
host fixture checks null and distinct nonnull results, confirms the first
getter ignores either null or nonnull input, and compares full input/owner
images and the complete pointed-to context-array image across calls. The
second getter uses a separate opaque-object sentinel because no pointee type is
established by its instructions. The matching build now uses this C source in
place of the copied 24-byte block, with copied prefix and suffix wrappers at
the original ROM boundaries. `asm/external_symbols.s` supplies the two
absolute RAM object aliases; the former copied code alias was removed.
