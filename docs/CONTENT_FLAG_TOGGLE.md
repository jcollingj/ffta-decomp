# Content global flag toggle candidate

The raw USA ROM block is `[0x96DE4, 0x96E18)` (52 bytes), SHA-256
`a43cddf9b21d014a025d418cb842ba166612d46dc1555ec9181d68e14625b421` and
SHA-1 `5cb4b550737d4d9788535cbf6e7058da6fc9cf25`. The complete ROM SHA-1 is
`4ac05441f4de70a4ec3dd932116346c61b8783d9`.

At `0x96DE4`, Thumb code shifts the incoming value left by 24 and tests for
zero, so the low input byte determines the branch. It loads the pointer stored
at RAM `0x0200F4A8` once before the branch. Both paths use that captured owner
pointer and read the word at target offset `+0xD0`: a nonzero input clears bit
`0x2000` with mask `0xFFFFDFFF`, while zero sets bit `0x2000`. The selected
word is stored back through the captured pointer. The code symbol is 50 bytes;
the final two bytes at `0x96E16–0x96E17` are zero alignment padding.

`src/content_flag_toggle.c` uses the existing
`ContentGlobalPointerOwner` declaration and natural C control flow. Its
separate-translation-unit host harness checks all 256 input bytes against
fixed flag vectors and compares complete owner-plus-canary images. A second
owner switch verifies the global pointer is loaded for each invocation. The
harness also calls the existing `GetContentContextPointerAt4` implementation
to exercise the shared owner declaration across translation units. These are
host semantic checks, not gameplay evidence.

Against immutable `build/verified-286-content-ref.elf` (SHA-256
`463c56de5fa6e3ea3a512b3d319483ee254af625a38b42894083a384f3b917f2`), the
isolated probe matches all 52 bytes, including the two zero padding bytes.
The candidate is admitted to the matching build as a 50-byte function within
the 52-byte C region; the suffix wrapper begins at `0x96E18` and preserves the
following copied bytes.

Reproduce the focused checks with:

```sh
python3 -m unittest tests.test_content_flag_toggle_c -v
python3 tools/decomp/probe.py src/content_flag_toggle.c \
  --start 0x96de4 --end 0x96e18 \
  --reference-elf build/verified-286-content-ref.elf --disassemble
```
