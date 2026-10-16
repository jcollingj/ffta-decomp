# Content output append callback

The callback at ROM file interval `[0x9AC20, 0x9AC30)` is 16 bytes long. The
original bytes are `4a 68 01 c2 08 68 01 30 08 60 4a 60 01 20 70 47`.
Thumb decoding gives:

| Address | Operation | Effect |
| --- | --- | --- |
| `0x9AC20` | `ldr r2, [r1, #4]` | Load native output cursor |
| `0x9AC22` | `stmia r2!, {r0}` | Store incoming pointer-sized value and advance cursor |
| `0x9AC24` | `ldr r0, [r1, #0]` | Load 32-bit count |
| `0x9AC26` | `adds r0, #1` | Increment count modulo 2^32 |
| `0x9AC28` | `str r0, [r1, #0]` | Save count |
| `0x9AC2A` | `str r2, [r1, #4]` | Save advanced cursor |
| `0x9AC2C` | `movs r0, #1` | Return continuation value 1 |
| `0x9AC2E` | `bx lr` | Return |

`ContentAppendState` models the target count at offset 0 and pointer cursor at
offset 4. Its cursor is native `void **`; the generic callback stores the
incoming pointer as a `void *`, preserving the host pointer width. The caller
path at `0x99D08` obtains a payload through a callback pair before reaching this
routine, so this generic callback does not directly match `ListVisitCallback`.
The matching function inventory records `ContentAppendCallback` at file
interval `[0x9AC20, 0x9AC30)` in Thumb mode, so the standard comparison checks
its symbol address, size, and bytes.

The host test compiles the production source as a separate translation unit. It
compares complete fixture images after each of three successive pointer
appends, checks a `0xffffffff` count wrap with a null value, and uses
sentinel-filled output neighbors to detect writes outside the output area.
