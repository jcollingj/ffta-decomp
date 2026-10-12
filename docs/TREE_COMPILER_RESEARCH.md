# Tree traversal compiler-shape research

This note records the bounded Luna session 26 search for natural C shapes matching `CountTreeNodes` (`0xC7B20–0xC7B74`) and `CountTreeRootNodes` (`0xC7DEC–0xC7E38`). These remain nonmatching semantic candidates; none of these probes are integrated or counted as recovered code. Probes used the existing agbcc `-O2` setup, no reference ELF for these leaf sources, and no register-binding or opcode assembly.

## Original traversal evidence

The root routine's 74 function bytes begin with `push {r4,r5,lr}`. It copies the input to `r1` (current node), initializes `r4` to zero (count), initializes `r3` to -1 (previous node), then saves the root from `r1` in `r5`. The loop loads `child_b` at offset 16. It descends there if nonzero and not the previous node, unless `child_a` equals the previous node; otherwise it examines `child_a` at offset 12, increments the count, and descends if nonzero and not the previous node. Return-to-parent loads offset 8. Its three back-edges are child-B descent to `0xC7DFE`, child-A descent to `0xC7DFE`, and parent return to `0xC7DFA`; the parent edge first sets `previous = node`. The root comparison exits rather than ascending beyond the requested subtree. Bytes `0xC7E36–0xC7E37` are two zero alignment bytes after the 74-byte function.

The initial direct-root candidate used `r4` as root and `r5` as count, while retaining the same node/previous roles. Its symbol size and padding were correct, but 12 function bytes differed. Reordering declarations alone had no effect. Removing the redundant `root = node` source assignment also had no effect. The source initializer order did matter: initializing count and previous before node moved the result to six differing bytes while retaining the correct symbol and block sizes. The remaining differences are consistent with root/count register allocation and the placement of the root save; this is a compiler observation, not a claim that source C names or declarations bind registers.

## Bounded variants

Each row is a direct-root `CountTreeRootNodes` source variant, independently compiled and probed at the same function address; the `CountTreeNodes` owner candidate was left at its starting source throughout this shape search. The SHA-256 is for that exact scratch source under `build/probes/tree_shape_sources/`.

| Variant | Source-level hypothesis | Symbol / target bytes | Function byte differences | SHA-256 |
|---|---|---:|---:|---|
| v01_stop_local | Save the root in a distinct `stop` local | 74 / 74 | 12 | `bd4720fb41abefc2bc9792a272c72d596bc77d76a009a7daa5eb7aa4bb61f529` |
| v02_null_early | Return on null, then use an unconditional loop | 78 / 74 | 72 | `7fe827bfe570484c9dfac954d8276ed40fd3d1a617245e1e21f28646937954d9` |
| v03_for_init | Put node/count/previous setup in a `for` initializer | 74 / 74 | 12 | `0c390c5c898045056fe72deadc5e7f4c7617eb2a7afccf2f997caaca39f3720d` |
| v04_init_order | Initialize count, previous, then node | 74 / 74 | **6** | `9aa7f5de8da1de21fbbc0e5b33a6c884b8abbf4b0e43ed620290cadd1a167a20` |
| v05_prev_last | Keep node/count/previous order (control) | 74 / 74 | 12 | `0c0a366ce49d5322ed371710a36bc35d5230df5bc4a89bd886855fd5707381e9` |
| v06_block_init | Put initializations in a nested block, count first | 74 / 74 | 11 | `9f0ce92a369aa8b1a25fd3f7b970992020b8ea193f72c9b17f3c2094fb3c9e68` |
| v07_child_select | Select a `next` child in one shared branch | 64 / 74 | 62 | `f834d87577a85dcb53c140fa1713837a1887fac8c1747e3313c525c6c50f91e0` |
| v08_sentinel_uint | Spell the previous sentinel as unsigned all-ones | 74 / 74 | 12 | `15e4aee96ebf76a29bd5f2db4e2edb445007acb19e341d788dbc506e3821acb1` |
| v09_static_helper | Factor the walk into a static helper | wrapper placement shifted by `0x4C`; 88-byte unit | 73 block differences | `bd8dd3ca3f4cedf549e4cfbf47cdaa7a152e767dd22443d0ae30ed8eeca461ce` |
| v10_root_condition | Replace the root break with a terminating node assignment | 76 / 74 | 66 | `6a37d3f495404a818ba034eefd85b4aa6209ab6757cbb04159d2739d4debc650` |
| v11_hoist_children | Load both child pointers before branching | 72 / 74 | 54 | `f170d9aa372da80dbd83b580cc88c838917f0f1fcf0f23120899065ebc55eca4` |
| v12_do_null_guard | Guard null then express traversal as `do/while` | 82 / 74 | 75 | `29957f37223dae5997e54cdbaa26496c590bbf2ad4af50610db664aee07b4e5a` |

The helper-factor variant places its helper before the target wrapper, so its reported target symbol does not begin at the requested interval; that result is only a placement warning and provides no matching evidence. The six-difference `v04_init_order` source is preserved as `src/nonmatching/tree_root_count.c`.

## Current candidates and checks

Current source SHA-256 values:

- `src/nonmatching/tree_count.c`: `fcee1a220aa6028fb26087cee7c1549a2bfbdb1e42ef586e717814eded1019c3`
- `src/nonmatching/tree_root_count.c`: `9aa7f5de8da1de21fbbc0e5b33a6c884b8abbf4b0e43ed620290cadd1a167a20`

The current isolated probes report `CountTreeNodes` at the correct address with an 82-byte symbol versus the 84-byte function target and 65 differing bytes. `CountTreeRootNodes` is at the correct address and has the correct 74-byte symbol plus two zero-padding bytes, with six differing function bytes. `nm -S` confirms symbol sizes `0x52` and `0x4A` respectively.

`python3 -m unittest tests.test_tree_count_c tests.test_tree_root_count_c -v` passes both host harnesses. They cover null and single-node roots, left/right skew, balanced and mixed trees, a 513-node count, arbitrary subtree roots with external parent/sibling nodes, and full fixture immutability. These tests support the candidate semantics; original-byte comparison remains the acceptance oracle.
