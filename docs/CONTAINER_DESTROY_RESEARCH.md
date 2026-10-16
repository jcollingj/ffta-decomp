# Combined container destruction research

## Bounds and dependencies

`DestroyContainer` is the Thumb routine at ROM file offset `0xC7908`, with
exclusive end `0xC799C`. This is a 148-byte executable block; the ending
`bx r1` is at `0xC799A` and there is no padding in this interval. I inspected
`baserom.gba` directly with `arm-none-eabi-objdump -D -b binary -m arm
-M force-thumb --adjust-vma=0x08000000 --start-address=0x080C7908
--stop-address=0x080C799C`. The ROM SHA-1 is expected to be
`4ac05441f4de70a4ec3dd932116346c61b8783d9`.

The dependency probe used `build/verified-278-tree-layout-ref.elf`, SHA-256
`be0a3ca7edfc3057225e11b125451376d250ea2b3c7cd52d0953a02276fbc2e4`. The
source calls actual `FreeListNodes` in its own translation unit and existing
`AllocatorFree`; the current native harness will retain allocation storage in
the allocator stub.

## Original instruction map

| ROM offset | Instruction/effect | Control flow and load timing |
| --- | --- | --- |
| `C7908–C790E` | Save low registers, move prior `r9/r8` values to saved low registers, then save them | Entry prologue. |
| `C7910–C7914` | Copy `owner` from `r0` to `r5`, then `r9`, then `r7` | `r5` is used for the container during list/tree work, `r9` is retained for the final allocator reload, and `r7` is reused for list/tree allocator captures. |
| `C7916` | Load `[owner]` into `r6` | Capture list allocator before any callback. |
| `C7918` | Load `[owner+4]` into `r4` | Capture list head. |
| `C791A–C791C` | Compare head; `beq C7932` | Null head skips list frees and reaches list-root clear. |
| `C791E` | Load `[head+8]` into `r1` | Reads successor before any callback. |
| `C7920–C7922` | Compare successor; `beq C792A` | Null successor skips recursive suffix free. |
| `C7924–C7926` | `r0 = captured allocator`; call `FreeListNodes` | Recursively frees suffix. |
| `C792A–C792E` | `r0 = captured allocator`, `r1 = captured head`; call `AllocatorFree` | Frees the captured original list head. |
| `C7932–C7934` | Store zero at `[owner+4]` | Clear list root only after all list callbacks. |
| `C7938` | Reload `[owner]` into `r7` | Capture tree allocator after list callbacks and list-root clear. |
| `C793A` | Load `[owner+8]` into `r4` | Capture tree root after list callbacks. |
| `C793C–C7940` | `previous = -1`; `root = node` | Tree walk initialization. |
| `C7942` | Compare current node; `beq C797C` | Null tree skips loop and clears root. |
| `C7946–C7948` | Load child A/B at offsets 12/16 | Both child values are captured before traversal decision. |
| `C794A–C7954` | Child A non-null, not previous, and child B not previous | On success choose A. Entry goes directly to `C795C` if A null; the equality branch instructions at `C7950` and `C7954` also reach `C795C`. |
| `C7956–C795A` | `previous = node`; `node = child A` | Back-edge `C795A -> C7942`. |
| `C795C–C7962` | Child B non-null and not previous | If selected, set previous/current and back-edge `C7968 -> C7942`; otherwise fall through to free. |
| `C796A–C796E` | Call `AllocatorFree(captured tree allocator, current node)` | Node is freed before root comparison and parent read. |
| `C7972–C7974` | Compare current node to captured root; `beq C797C` | Root is still held in `r6`; root ends the traversal. |
| `C7976–C797A` | `previous = freed node`; load `[freed node+8]` into current | The parent link is reloaded **after** the free callback. Back-edge `C797A -> C7942`. |
| `C797C–C7980` | Set zero, copy owner from `r8`, store zero at `[owner+8]` | Clear tree root. |
| `C7984` | Reload `[owner]` into `r4` | Capture allocator after both root clears and immediately before owner free. |
| `C7986–C798A` | Pass reloaded allocator and owner to `AllocatorFree` | The owner allocation is freed last. |
| `C798E–C799A` | Copy captured allocator into return register and restore/return | Returns the allocator pointer loaded before owner free, even if callback mutation changes the owner field. |

The source candidate preserves the different list/tree allocator capture
points, the root clears after callbacks, the post-free parent reload, and the
final owner-allocator reload. These timings make callback mutations observable.

## Initial probe and compiler hypotheses

The first natural source used a single owner pointer, inline list-head release,
and an inline postorder tree walk matching `FreeTreeNodes`. Its function
symbol was 126 bytes in a 148-byte interval (141 differing interval bytes).
Twelve source-shape trials were compiled with pinned agbcc `-O2` against the
immutable ELF above. Literal trial source files are tracked in
`tools/research/container_destroy_trials/`; hashes identify exact source bytes. Symbol
sizes and interval differences come from each trial's `comparison.json`.

| Trial | Natural-source hypothesis | Source SHA-256 | Candidate symbol bytes | Overlap byte differences | Full 148-byte extent differences |
| --- | --- | --- | ---: | ---: | ---: |
| v01 | Baseline initialized locals; guarded do/while | `c4d1cf7d427fd7d55cf4abc32152af12aa5e10f19e84c732101c7e19c0500aa8` | 126 | 119 | 141 |
| v02 | Distinct owner aliases for list, tree, and final free stages | `5c9ad09a92824627467635562165daadddda06a6ace27e473ca3fd94d660c3ec` | 140 | 128 | 136 |
| v03 | `while` loop in place of guarded do/while | `7ef3db6102fea109a6f646e342e8373e63f43127fe7f5e9f10639138c3c9a3da` | 126 | 119 | 141 |
| v04 | Declare list locals uninitialized and assign sequentially | `89aa3a4147bcded0ce1d9d2730581a8dfdae25627ae59c3f19ee4007f4890617` | 122 | 115 | 141 |
| v05 | Capture tree root directly from owner separately from current node | `8eafa6388b5da4068c16dccd86c51f1d6cc7dde66a16a608eef95541dca411f4` | 126 | 119 | 141 |
| v06 | One owner alias used consistently | `bfb29254f4e8d119630655765c0d5ccd8079dfed73671fad1716d0189e346f58` | 126 | 119 | 141 |
| v07 | Reverse root and previous initialization order | `abd131753fd4ef09f1a4eb1ac0d5136afd167d3323d4dc974e0b5f9b5b0ecf6c` | 126 | 119 | 141 |
| v08 | Nest the child-A conditions | `d570212003c6b0f082e014a7bea6d88e5a2f88dc878d5bddf671f605eaf9b3b1` | 126 | 119 | 141 |
| v09 | Reorder declarations while preserving initialization order | `a3719808aae954230d03073946f49cebbbc9e84ad16c43d155d1f75aaa724ff2` | 126 | 119 | 141 |
| v10 | Read the parent into a local after `AllocatorFree` | `f7fe60234cf15efcee6740d5f14d81aed63c59cbbfa7364e505005c6a5c01946` | 126 | 119 | 141 |
| v11 | Make the local owner pointer parameter volatile | `71dbfc1524eaefc32f07994b22867bff59071d30441b716fccfd83cb7a5389f8` | 144 | 66 | 70 |
| v12 | Reassign one owner local at each lifecycle stage | `fe4945047a2598f3cc5886b9d9a114c6a4cb4372bd383e79180f7d21fc5ea5a8` | 142 | 131 | 137 |

The maintained best ordinary-C candidate is v02 (140 symbol bytes, 128 byte
differences over its 140-byte overlap, and 136 differences over the full
148-byte reference extent). v11 is a rejected diagnostic experiment: its top-level
volatile owner parameter was introduced only to influence register allocation,
with no original evidence supporting volatile access. The function type is
compatible and its host behavior passed, but matching bytes alone do not
justify that source choice. It has 144 symbol bytes, 66 differences over the
overlap, and 70 over the full reference extent. Neither candidate matches; no
matching code credit is claimed.


## Review corrections and reusable lesson

Review found that the first harness did not assert the function return value or
the complete final owner image. The revised fixtures assert the returned
allocator in empty, list-only, tree-only, and combined cases. On the final owner
free in the combined case, the retained-storage stub changes the owner
allocator from C to D; the function must still return its previously captured C
while the final whole-owner image contains D and both roots are null. The
expected final image is built separately from candidate execution. This catches
a return value incorrectly reread from freed owner storage.

Review also corrected instruction annotations against the raw listing: owner
register copies are at `C7910–C7914`, branch instructions are at `C7950` and
`C7954` (their comparison instructions are `C794E` and `C7952`), and the final
allocator load is at `C7984`. For pointer-heavy routines, a pre-implementation
fixture plan should explicitly include the function's return value, callback
mutation at the final free, and the complete owner image after that callback.
