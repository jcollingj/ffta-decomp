# Content global flag modifiers

The bounded USA ROM block is `[0x96D94, 0x96DE4)` (80 bytes), SHA-256
`b276aa0638ae1b4d7144d9c5c9ccf481adf0881cdd4f3bfc173b69fc583e8cf8` and
SHA-1 `b3c5fd3743dc9bc45da8f21a07077a3e84c4247e`. The complete USA ROM SHA-1
is `4ac05441f4de70a4ec3dd932116346c61b8783d9`.

All three routines load the pointer held at RAM `0x0200F4A8`, then update the
32-bit word at owner target offset `+0xD0`. The first clears bit `0x0800` and
sets bit `0x1000`; the second sets mask `0x8800`; the third clears mask
`0x8800`. Each routine stores the result and returns. Their literal pools
contain `0x0200F4A8` at `0x96DAC`, `0x96DC8`, and `0x96DDC`; the first routine's
clear mask is `0xFFFFF7FF`, and the final routine's clear mask is `0xFFFF77FF`.

The existing owner view now keeps `unknown04` as a native pointer, pads the
known intervening bytes, and exposes `flagsD0`. The target prefix size is
`0xD4` bytes. On a typical 64-bit host the pointer shifts `flagsD0` to offset
`0xD8`; host tests assert the native offset relationship rather than assuming
target and host offsets agree.

The matching C source is `src/content_global_flags.c`. Against
the immutable verified-283 reference ELF (SHA-256
`680c2cffcfacb1ccbd7931d1b7cf388c088fce9776c3490370ffa13a9b6f26f0`), the
isolated probe reports 80/80 bytes, zero differing bytes, and matching symbol
addresses and sizes: 32, 24, and 24 bytes. The exact block and functions are
admitted to the matching build and region ledger at the original addresses.
After integration, the rebuilt ROM SHA-1 remains
`4ac05441f4de70a4ec3dd932116346c61b8783d9`.

The separate-translation-unit host test links these modifiers with the actual
`src/content_globals.c` getter. Fixed expected outputs cover each input bit,
the changed mask bits, dense/composite values, and a sequence of all three
operations. It compares complete owner and trailing-canary images after each
mutation and switches the global to a second owner to verify reload behavior.
These checks establish host semantics and composition, not gameplay behavior.

Reproduce the focused checks with:

```sh
python3 -m unittest tests.test_content_global_flags_c tests.test_content_globals_c -v
python3 tools/decomp/probe.py src/content_global_flags.c \
  --start 0x96d94 --end 0x96de4 \
  --reference-elf build/verified-283-content-ref.elf --disassemble
```
