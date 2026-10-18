# Numeric-key tree rebuild candidate

`RebuildTreeByNumericKey` is an unadmitted source candidate for the original
120-byte interval `0x0C7C48..0x0C7CC0`. The routine repeatedly traverses the
tree in reverse order with `ListCallbackSelectClear`, detaches each selected
node, sets flag mask `1`, and reinserts it by signed word-0 key. Equal keys
follow child B. When no clear-flag node remains, it traverses again with
`ListCallbackClearFlag0`. The final callback clears only mask `1`, preserving
all higher flag bits.

## Original instruction evidence

The raw Thumb slice was decoded from the checked original ROM with:

```sh
python3 - <<'PY'
import subprocess
from pathlib import Path
b = Path('baserom.gba').read_bytes()[0xC7C48:0xC7CC0]
p = '/tmp/tree-rebuild.bin'
Path(p).write_bytes(b)
print(subprocess.run([
    'arm-none-eabi-objdump', '-D', '-b', 'binary', '-m', 'arm',
    '-M', 'force-thumb', '--adjust-vma=0xc7c48', p,
], text=True, capture_output=True, check=True).stdout)
PY
```

Entry `0xC7C48` saves r4-r6/lr and reserves one stack word. The literal loaded
at `0xC7C50` is `0x080C7E79`, the Thumb address of
`ListCallbackSelectClear` at `0xC7E78`; the literal at `0xC7CBC` is
`0x080C7E95`, the Thumb address of `ListCallbackClearFlag0` at `0xC7E94`.
The first direct BL at `0xC7C5A` calls `0xC7858` (`DetachTreeNode`, still an
uncredited copied-code research candidate). The two traversal BLs at
`0xC7C9E` and `0xC7CAE` both target `0xC7D70`
(`TraverseTreeReverseInOrder`). These direct call targets are distinct from
the callback literals.

The selected pointer is first written into the stack slot by the callback.
The routine tests that slot, calls `DetachTreeNode`, then reloads the selected
pointer at `0xC7C5E`. It ORs `1` into the node flags and compares the node's
word-0 key as signed at `0xC7C78`. A negative comparison follows child A at
`+12`; equality and greater values follow child B at `+16`. The selected node
link is stored before its parent link at `+8`. An empty owner root receives
the selected node directly. A nonzero selected pointer repeats the traversal;
zero selects the final flag-clear traversal. The source candidate preserves
the post-detach reload by using the callback's `void *` storage directly.

## Behavioral evidence

The focused host test compiles the candidate, traversal, callbacks, and detach
dependency as separate translation units. Its controlled tests use a leaf-only
detach stub to isolate selection and insertion. Fixed whole-owner and complete
node-array expectations cover an empty tree, a singleton, signed extrema,
equal keys routing to child B, a preexisting mask-1 node, and retained high
flags. The stub records the order of detached leaves.

A separate composition executable links the real reverse traversal and both
real callbacks with the uncredited `DetachTreeNode` candidate. Its initial
three-node tree has root key 50, child-B key 10, and that node's child-B key 70,
so the links are valid but their numeric ordering is stale. The separately
measured initial reverse traversal visits nodes in order 70, 10, 50. The
hand-derived rebuild selects 70 first, reinserts it after a two-edge search,
then selects 10; detaching 10 moves 70 under root key 50 and inserts 10 at
child A. The final full node-array image verifies all parent/child links and
flags. The rebuild's internal callback order is derived from the fixed input
and transitions; the harness does not instrument those internal invocations.
Candidate composition is behavioral evidence only and does not credit
`DetachTreeNode` or this rebuild candidate with original bytes.

Run the focused host cases with:

```sh
python3 -m unittest tests.test_tree_rebuild_c -v
```

## Matching status

The current natural C candidate has SHA-256
`9507bfb4406d74572de67b84f1a6518d0cb509b2f435f6f1f110d200a6020105`.
Probe it against the immutable 319-function reference whose SHA-256 is
`bcf80732bc56510d68bddc2939221bb54cbc2788a641005cc8212ae940d47bd2`:

```sh
python3 tools/decomp/probe.py src/nonmatching/tree_rebuild.c \
  --start 0xc7c48 --end 0xc7cc0 \
  --reference-elf build/verified-319-content-ref.elf --disassemble
```

The symbol and target interval are both 120 bytes; the probe reports 26
differing bytes, first at `0xC7C76`, and `accepted: false`. This is not a
matching function and receives zero recovered-byte/function credit. The
probe's `UNINVENTORIED` status is expected for this candidate and is not a
reviewed function inventory entry. The nearby copied `DetachTreeNode` alias
remains uncredited. No matching region or function ledger has been changed.

### Bounded natural-source trials

All trials below use GCC 2.9 at the probe's `-O2` setting, the same immutable
reference hash above, and the same `0xC7C48..0xC7CC0` range. The symbol-aligned
size and difference counts are read from each probe's `comparison.json`; every
trial is nonmatching. The current source is the lowest-difference retained
form.

| Trial | Source transform | Source SHA-256 | Symbol size / differing bytes |
| --- | --- | --- | ---: |
| `common_child_while` (retained) | Final source: `void *` selection storage is read after detach; signed key is cached; the owner alias is used for root insertion; a local child pointer joins both search branches before cursor advancement; loop spelled `while (1)`. | `9507bfb4406d74572de67b84f1a6518d0cb509b2f435f6f1f110d200a6020105` | 120 / 26 |
| `common_child_for` | From retained source, replace only `while (1)` before `struct TreeCountNode *child;` with `for (;;)`. | `2d2e22c3aa1c65ec5aa64dc3d9fc8c9bb3aef6c3ec23aa461077ca083e15c31a` | 120 / 26 |
| `inverted_child_tests` | From retained source, replace the common-child loop body with the following direct nonnull-advance form. | `aeb84719fd997b91655415b4e7500b96517f9b893b87196668f746bd73b81d75` | 120 / 73 |
| `typed_selected_before_detach` | Reconstructed first-draft form assigns `struct TreeCountNode *selected = selected_value` before the null check and detach, then uses `selected` for flags, comparisons, and links; it does not cache the key, retain a separate owner alias, or join child selection through a local. Its measured hash is for this reconstruction; the original transient draft hash is unknown. | `b55fdf20e4e6e228165b83f21b3359f1054f81aeb49a1d64302f319f7fc05576` | 120 / 54 |

The exact body used for `inverted_child_tests` is:

```c
if (key < (int)cursor->unknown00) {
    if (cursor->child_a != 0) {
        cursor = cursor->child_a;
    } else {
        cursor->child_a = selected_value;
        break;
    }
} else {
    if (cursor->child_b != 0) {
        cursor = cursor->child_b;
    } else {
        cursor->child_b = selected_value;
        break;
    }
}
```

The retained loop spelling is not a compiler hint; replacing it with `for (;;)`
produced identical bytes. The typed-before-detach draft is kept only as a
measured historical comparison and is not the final source. No volatile
objects, inline instruction assembly, or register constraints were used.
