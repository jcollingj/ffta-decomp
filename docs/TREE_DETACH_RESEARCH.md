# Tree node detach/reinsert research

This is an unadmitted candidate for ROM file offset `0x0C7858..0x0C7908`.
The symbol occupies `0x0C7858..0x0C7906` (174 bytes); the final halfword at
`0x0C7906..0x0C7908` is zero padding. The original ROM identity was checked by
`tools/decomp/probe.py` against SHA-1
`4ac05441f4de70a4ec3dd932116346c61b8783d9`.

## Original control flow

The raw Thumb listing was independently produced from the ROM slice
`baserom.gba[0xC7858:0xC7908]` with `arm-none-eabi-objdump -D -b binary -m arm
-M force-thumb`. The listing's local offsets are added to `0xC7858` below.
The complete command output and bytes are reproducible from that slice; the
two terminal zero bytes are outside the symbol.

| Address | Instruction/effect | Flow evidence |
| --- | --- | --- |
| `C7858` | push r4,r5,lr; owner to r5 | Entry |
| `C785C` | load node parent | `C7860` tests it; `C7860` BNE `C7886` selects nonroot path |
| `C7862` | clear owner root | Root path starts with this store |
| `C7864..C7878` | prefer child A; else child B; clear only the selected node link | `C7868` BEQ `C7870`; child-A case branches `C786E` to `C787A`; child-B null falls through |
| `C787A..C787E` | reload owner root, if nonnull clear its parent | `C787E` BEQ `C78A0` |
| `C7886..C789A` | compare parent child A with removed node, else compare child B; clear matching link | `C788A` BNE `C7892`; `C7890` branches `C789C`; `C7896` BNE `C789C` |
| `C789C..C78A0` | set r0 to zero and clear removed node parent | Nonroot unlink completion |
| `C78A0` | load removed child A | `C78A4` BEQ `C78CE` skips the entire first insertion |
| `C78A6..C78C8` | search from the *current owner root* by signed word-0 key; `<` follows A, otherwise B; write the empty edge and subtree parent | Search comparison at `C78AE`; `BGE C78BC` selects B; `C78B6 -> C78C2` joins cursor advancement and `C78C4 -> C78AC` is the backedge; install and parent store at `C78C6..C78C8` |
| `C78CC` | clear removed child A | Reached only after child A insertion |
| `C78CE` | load removed child B | `C78D2` BEQ `C78FC` skips second insertion |
| `C78D4..C78F6` | repeat the same signed-key search from current owner root | `BGE C78EA` selects B; `C78E4 -> C78F0` joins cursor advancement and `C78F2 -> C78DA` is the backedge; install and parent store at `C78F4..C78F6` |
| `C78F8` | set r0 to zero | Prepares removed-child-B clear |
| `C78FA` | clear removed child B | Reached only after child B insertion |
| `C78FC` | set r0 to zero | Prepares common parent clear |
| `C78FE` | clear removed node parent | Common exit, including root path |
| `C7900..C7904` | pop and return | Function ends at `C7906` |

The zero-, one-, and two-iteration paths are explicit: each insertion's initial
BEQ bypasses its loop; each nonempty subtree enters at the root load and key
comparison; either empty-edge store exits that insertion, while a nonempty
edge takes one of the two backedges. Child A completes before child B starts.
This is the original literal detach-and-reinsert procedure. It is not a
successor/predecessor textbook deletion, and it makes no allocator call. Its
pointer assumptions are limited to the code's checks: for a nonroot node it
clears child A if equal, otherwise clears child B if equal, then clears the
node parent. It does not repair an unrelated malformed parent link.

## Candidate probe record

The host suite passed at the time of this record. The target candidate remains
DIFF and must not be counted as recovered code. Candidate source SHA-256:
`79c3174949b6ddd94bbd5574fdced852d4ca84c0c2e04d23434abfa7783ca298`.
Command:

```sh
python3 tools/decomp/probe.py src/nonmatching/tree_detach.c --start 0xC7858 --end 0xC7908 --disassemble
```

At `-O2`, the candidate symbol is 174 bytes and the original block interval
is 176 bytes including padding. The symbol-interval comparison reports 137
different bytes; the complete-block comparison also reports 137 (the two pad
bytes match). The candidate `nm` symbol is unreviewed, so the interval metric
is an independently calculated extent comparison rather than an admitted
function match. The candidate has no external references. The semantically
tested `v02_inverted_root_test` form is retained: swapping the root and nonroot
branches preserves the complete fixed-state behavior and reduces both byte
difference counts from 145 to 137 without changing the 174-byte symbol size.

Eight isolated natural-source compiler hypotheses were tried from the original
`6e3a...` candidate baseline. The exact baseline is reconstructed from the
retained v02 source by applying the inverse branch-block replacement. Literal
edit anchors and replacement text,
expected source hashes, bounded probe command, and metric checks are executable
in `tools/research/tree_detach_trials.py`. Reproduce all trials with
`python3 tools/research/tree_detach_trials.py --probe`. The script rebuilds each
variant from the tracked baseline and fails if any reconstructed source hash,
function metric, block metric, or nm symbol size differs. None of these variants
replaced the retained production candidate.

| Trial | Natural source change | SHA-256 | Function diff / 174-byte original | Symbol size | Full 176-byte block diff |
| --- | --- | --- | ---: | ---: | ---: |
| `v01_braces` | Add braces to the two matching parent-link branches | `bf23dbd4f6981e6a13f57415585f979000cfe62e7665e0576c8bbd9ccc81f722` | 145 | 174 | 145 |
| `v02_inverted_root_test` | Express the nonroot case first, with the equivalent root case in `else` | `79c3174949b6ddd94bbd5574fdced852d4ca84c0c2e04d23434abfa7783ca298` | 137 | 174 | 137 |
| `v03_while_loop` | Spell each unbounded search loop as `while (1)` | `81d1e7c32186d1935759067325e6ccd84fa188b0eddb10cf0f56f6555d50baaa` | 145 | 174 | 145 |
| `v04_signed_spelling` | Spell the key casts as `signed int` | `46b7abf5954429dc181798dd43ea44cf5bf780be84c3963d1c00b13ccb8e1f4e` | 145 | 174 | 145 |
| `v05_link_pointer` | Select a `TreeCountNode **` edge, then load/store through it | `ea602b030cf183779c177ee242132b013de0d24cf58ea4cae00a6ea611167dac` | 161 | 168 | 163 |
| `v06_move_parent_clear` | Leave the nonroot parent clear to the shared final store | `c9e841289c06bfc537a8d2a112a09551cb322edc62985b7dbb3dee97d493de75` | 141 | 170 | 143 |
| `v07_child_local_assignment` | Use a scoped pointer to the chosen child link | `570473247f5d31dc963025a5cf3d10287320d6425765027790b531586aaca5fb` | 161 | 168 | 163 |
| `v08_pointer_compare_reordered` | Reverse equality operands in parent-link checks | `159ce89e6996c0bec0118c20b61b14d852ac1c4088ebefb4536029f082004326` | 145 | 174 | 145 |

These results are compiler-shape evidence only. In particular, variants that
alter intermediate writes or malformed-pointer behavior require renewed
semantic review before they can be retained. Block counts above include the
entire `0xC7858..0xC7908` interval, including the two-byte pad; symbol size is
reported separately.
