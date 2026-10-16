# Tree clear routine research

## Audited ROM interval

`FreeTreeNodes` is a Thumb routine at file offset `0xC79E0`. Its complete
interval is `[0xC79E0, 0xC7A34)`: 82 instruction bytes ending with `bx r0` at
`0xC7A30`, followed by two zero alignment bytes at `0xC7A32`. The direct audit
used `baserom.gba`; the matching ROM identity is SHA-1
`4ac05441f4de70a4ec3dd932116346c61b8783d9`.

The saved-register and field provenance is:

| Register | Value and lifetime | Evidence |
| --- | --- | --- |
| `r5` | Owner argument, retained through the final `tree_root = 0` store | `0xC79E2`, final store at `0xC7A2A` |
| `r7` | `owner->allocator_owner`, passed as free argument 1 | load `[r5]` at `0xC79E4`, copied to `r0` at `0xC7A16` |
| `r4` | Current node; passed as free argument 2 and then used again | loaded from `[r5,#8]` at `0xC79E6`; free argument at `0xC7A18` |
| `r2` | Previous node, initialized to all-bits-one; updated before each descent and after a free | `movs/neg` at `0xC79E8–0xC79EA`; updates at `0xC7A02`, `0xC7A10`, `0xC7A22` |
| `r6` | Original tree root, retained for the post-free root comparison | copy from `r4` at `0xC79EC`, compare at `0xC7A1E` |

The node links are loaded from the current node at offsets 12 (child A), 16
(child B), and 8 (parent). The caller owner root is at offset 8. The `bl` at
`0xC7A1A` resolves to `AllocatorFree` at `0x08007170` in reference ELF
`build/verified-272-search-ref.elf`, SHA-256
`831b31e5d45d948f9b300d7cca015712c07cb97df0b0b13edbffcad5598c601a`.

## Branch and loop map

The initial `cmp r4,#0` at `0xC79EE` branches to owner clearing at `0xC7A28`
when the root is null. Otherwise the routine loads both children at
`0xC79F2–0xC79F4`. The child-A path falls through to `0xC7A02` only when A is
non-null and neither A nor B equals `previous`; it updates `previous`, moves A
to `node`, then branches back to the loop test at `0xC79EE` from `0xC7A06`.
If A is null, A equals `previous`, or B equals `previous`, control reaches the
child-B test at `0xC7A08`. A non-null B unequal to `previous` is selected at
`0xC7A10–0xC7A12`, with its back-edge at `0xC7A14` to `0xC79EE`.

If neither descent is selected, the current node is passed to `AllocatorFree`
at `0xC7A16–0xC7A1C`. The routine compares the still-live node value with the
saved root at `0xC7A1E`; equality exits to `0xC7A28`. Otherwise it stores the
freed node in `previous`, reloads `node->parent` from offset 8 at `0xC7A24`
*after* the free call, and takes the back-edge at `0xC7A26`. Finally it writes
zero to owner offset 8 and returns. The post-call parent reload is observable
when a free callback changes that link, so it is preserved as a source-level
ordering constraint. The host test stub deliberately retains allocations to
avoid using freed storage in the test.

## Candidate and compiler evidence

The routine is integrated as `src/tree_clear.c` and admitted in the matching
build ledger. Its initial copied remainder was `asm/rom_tree_clear_suffix.s`, which
preserves file offset `0xC7A34` for length `0xD4` (212 bytes). The former
copied object `asm/rom_list_count_wrappers_suffix_suffix_suffix.s` was retired
after checking that its only active consumers were the Makefile and copied
region entry; neither has remaining references to it.

The final source uses declaration/assignment order `allocator_owner`, `node`,
`previous`, `root`, followed by `if (node != 0) do { ... } while (node != 0)`.
Its SHA-256 is
`ca36cd87dcbf62c76194c9f04884c404b90db11fc32e1ecb962ae10e949855a3`. Probe
`python3 tools/decomp/probe.py src/tree_clear.c --start 0xc79e0
--end 0xc7a34 --reference-elf build/verified-272-search-ref.elf --disassemble`
reported exact equality: 84 candidate bytes, 84 original bytes, zero differing
bytes. Candidate block SHA-256 is
`d63ef505942fb99ce51a5b451e1be469d33d8a0bb95679c9e1d5ea25023233d3`.

Eight persisted compiler-shape trials compared the same 84-byte interval. Each
used the indicated natural-source edit, retained the function semantics, and
was compiled with the pinned agbcc `-O2` probe. Hashes identify the exact trial
source; probe outputs are under ignored `build/probes/` directories.

| Trial source | Exact source-shape hypothesis | Source SHA-256 | Differing bytes |
| --- | --- | --- | ---: |
| `v02_allocator_first` | Declare locals uninitialized; assign allocator owner, node, previous, then root; retain `while` | `4f857276c5412487f6523a7aaf9df30a0dc6879cc52d98a8c3110b3cfae77c2d` | 54 |
| `v03_root_last` | Initialize allocator owner, node, previous, then root in declarations; retain `while` | `6684e74c8b0f0bb08781cfd735f1dff6b6c8b95d3086a7674a69c8bcb90b119e` | 54 |
| `v04_explicit_owner` | Save the owner argument in a local alias and use it for all owner accesses | `98354c62508b1059cd4decd8a4ce4ff882d8d61b56f933b0fe31efb44e232781` | 61 |
| `v05_temp_parent` | Load `node->parent` into a local after the free call before updating traversal state | `fe37e582f150cb1bbd10b1d7c6e353e150bcb52efc379b4587ac72565a8fc1b8` | 61 |
| `v06_no_child_temps` | Declare child locals first, then assign both links before the branch tests | `04c970ea4ce3ea1271becd812f05b25e88ac25e9bc7f53c95485558e37fb593b` | 61 |
| `v09_cached_links` | Cache the post-free parent in a local and then update `previous` and `node` | `ddc8027d854a1f94bf7abe3dd1e801fc8b72961d0bfcc7cb03a5efe70cfd350e` | 61 |
| `v10_separate_root_assignment` | Assign allocator owner and node first, then separately assign root from node | `3e82fcda1aeca6d7a134b510786f99919e058cc28308b5e881ff9dd30f3cb378` | 62 |
| `v11_do_while` | Use allocator-first sequential assignment, then guard a `do/while` with `node != 0` | `ca36cd87dcbf62c76194c9f04884c404b90db11fc32e1ecb962ae10e949855a3` | 0 |

## Behavioral checks and limitations

`tests/test_tree_clear_c.py` compiles the production source as a separate host
translation unit. Its harness checks fixed callback traces for empty,
single-node, asymmetric, and balanced trees. At every free callback it checks
the complete owner and node fixtures, including that the owner root is still
unchanged. A callback mutation redirects a node's parent and checks both the
resulting trace and the complete final node/decoy storage against an expected
buffer prepared independently of the function call.

The exact target byte comparison and host behavior checks independently
support the recovered semantics. The harness does not test actual allocator
release behavior, and this routine's tree traversal assumes structurally
valid parent/child links; malformed or cyclic structures are not established
to be supported. Integration verification is reported by the parent in the
project work log.

The historical tree-clear remainder was subsequently carved for list unlink. Consult `config/decomp_regions.json` for the current copied wrappers; integration-time filenames above are historical evidence.
