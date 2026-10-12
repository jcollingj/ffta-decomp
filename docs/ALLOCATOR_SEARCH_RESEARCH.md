# Allocator search research

`AllocatorFindNeighbors` is an isolated, unintegrated candidate for the Thumb
routine at ROM file offset `0x6C88`. The USA reference ROM SHA-1 is
`4ac05441f4de70a4ec3dd932116346c61b8783d9`.

## Bounds and data accesses

The function is `0x6C88..0x6D36` (174 bytes, end exclusive); `0x6D36..0x6D38`
is two zero alignment bytes. The function-byte SHA-1 is
`3f465a0ab059993e465a1386e200b9260100046b`.

It reads a little-endian 16-bit starting offset at owner `+2`; offsets are in
four-byte words relative to the owner byte buffer, and zero means null. Nodes
have a little-endian size at `+6`, previous offset at `+8`, and next offset at
`+10`. The second argument is narrowed to 16 bits. The third argument points
to output A and the fourth points to output B. In the original Thumb register
trace those are `r2` and `r3`; `r6` and `r7` retain them. Entry clears output B
then output A. The common return stores node3 to output A then node2 to output
B, so if both outputs alias, output B's value remains.

## Control-flow edge table

| Branch instruction | Condition and destination | Resulting path |
| --- | --- | --- |
| `0x6CA4` | Starting offset is zero: branch to `0x6CAC`; otherwise fall through to `0x6CA6` to form the pointer, then join at `0x6CAE` | Null start reaches the null-node return; nonzero offset becomes the starting node. |
| `0x6CB2` | Starting node is null: branch to `0x6D30`; otherwise fall through to `0x6CB4` | Null start returns with both outputs already zero. |
| `0x6CBE` | `requested > size(start)`: branch to `0x6D12`; otherwise fall through to `0x6CC0`, which branches to `0x6CDA` | Greater requests enter the next-link scan; equal or smaller requests enter previous-link traversal. |
| `0x6CCC` | `requested <= size(previous)`: branch to `0x6CDA` | Save `previous` and follow its previous link again. |
| `0x6CD8` | `requested <= size(saved node)`: branch to `0x6D2C` | Return `(previous, saved node)`. |
| `0x6CD8` fallthrough | Both comparisons were false; fall through to `0x6CDA` | Perform the same backward step. Preserve this edge in source. |
| `0x6CE6` | Previous offset is zero: branch to `0x6CEE`; otherwise fall through to `0x6CE8` to form the pointer, then join at `0x6CF0` | Null previous link becomes a null node pointer. |
| `0x6CF4` | Previous node is nonnull: branch to `0x6CC2`; null falls through to `0x6CF6`, which branches to `0x6D2C` | Continue the backward comparison, or return `(null, saved node)`. |
| `0x6D02` | `requested <= size(current)`: branch to `0x6D10` | Promote the next node to current and continue the forward scan. |
| `0x6D0E` | `requested <= size(next)`: branch to `0x6D2C` | Return `(current, next)`. Otherwise fall through to `0x6D10`, promote next, and continue. |
| `0x6D1C` | Next offset is zero: branch to `0x6D24`; otherwise fall through to `0x6D1E` to form the pointer, then join at `0x6D26` | Null next link becomes a null node pointer. |
| `0x6D2A` | Next node is nonnull: branch to `0x6CF8`; null falls through to `0x6D2C` | Continue the forward comparison, or return `(current, null)`. |

For a stable previous-link chain, the `0x6CD8` both-false edge is unreachable:
backward traversal deepens only after `requested <= size(previous)`, so the
saved successor has already satisfied `requested <= size(saved)`. This is an
invariant observation, not a reason to rewrite the literal edge. The analogous
`0x6D02` promotion is also preserved even though forward traversal reaches a
new current only after proving its size is below the request. The outputs often
bracket the request on size-sorted lists, but the routine follows its literal
starting-node and link decisions; it is not a global best-fit scan.

## Evidence and current candidate

`tests/test_allocator_search_c.py` compiles the candidate as GNU89 host C and
runs independent fixed vectors. The harness checks empty and one-node lists,
zero requests and sizes, exact first/middle/last thresholds, both directions,
an irregular list that distinguishes the starting-node direction decision,
an output alias, output sentinels, and whole-buffer immutability. Its five-node
fixture includes a `0x0100` word offset and sizes above `0xFF`; explicit bounds
checks keep every fixture node inside the 1200-byte buffer. The focused test
passes.

The current source is the best semantically tested natural C shape from twelve
bounded hypotheses. The rows report source SHA-1, candidate interval size
(including end alignment), and differing bytes against the 176-byte reference
interval. Each compilable variant passed the same host harness. A failed first
attempt at the backward-for-loop shape used an obsolete label and was not
probed; after removing that label it passed and is listed below.

The hypotheses are reproducible as these source transformations of the
candidate before its explanatory comment was added:

1. Reverse only the entry stores that clear outputs A and B. Alias semantics
   stay unchanged: both stores write zero, and the return still writes A then
   B so B's value remains.
2. Initialize `base` at declaration and remove its first-body assignment.
3. Replace the previous-offset ternary assignment with an explicit zero test
   assigning either null or `base + (offset << 2)`.
4. Apply the same explicit zero-test transformation to the next-offset
   assignment.
5. Add a static `ReadSearchWord(const u8 *)` returning bytes 0 and 1 as a
   little-endian `u16`, then replace the owner, size, previous, and next field
   expressions with calls at offsets `2`, `6`, `8`, and `10`.
6. Replace the forward label's backward `goto` edges with a `for (;;)` body:
   null-next and the successful next-size comparison `break`; current-size
   success promotes next and `continue`s; otherwise promote next and iterate.
7. Replace backward `goto backward` edges with a `for (;;)` body, turning the
   first-comparison branch and literal both-false fallthrough into
   `node2 = node3; continue`.
8. Use the same backward `for (;;)` shape but retain the natural `while`
   condition and body structure, with the null-previous case going to output.
9. Replace each `(offset << 2)` with `(offset * 4)`.
10. Replace the disjoint low-byte OR high-byte composition with addition for
    owner, size, previous, and next words.
11. Add a `u16 size` local and assign the initial and backward-previous size
    expressions to it before comparing.
12. Invert the initial direction test into `if (requested <= size(start))`
    followed by an explicit backward entry and a forward `goto`.

| C-shape hypothesis | Source SHA-1 | Candidate bytes | Different bytes |
| --- | --- | ---: | ---: |
| Reverse entry output-clear order | `2eae2bb9bf09f3205852a699da0170803c6d0086` | 160 | 151 |
| Initialize base pointer at declaration | `c0be732b923a6ec3657639145dc321a9f0d3c6f5` | 160 | 149 |
| Explicit null branch for previous offset | `02d9d744a626617ed5f2e05366f3cfd46f6e2f03` | 160 | 149 |
| Explicit null branch for next offset | `fdf4ffb6de1e5fab41bbe84478eab00f5637b2aa` | 160 | 149 |
| Static little-endian read helper | `9d9cf7943479dcc6c90d2b52f242174d27e5504b` | 208 | 205 |
| Forward scan expressed as `for (;;)` | `bfd9cd433977d41aecb1ee3d5beda7efe4ca6222` | 160 | 142 |
| Backward scan expressed as `for (;;)` | `91e0cb833548199163c4399369aa6ac5b5cb34e6` | 160 | 147 |
| Backward scan expressed as `while` | `386ac7e5a40975aec7d2e9322d8a7019f30729cf` | 168 | 151 |
| Offset scaling written as multiplication by four | `19d26569e7564af36e33c1668a22180a792a1f2d` | 160 | 149 |
| Little-endian composition written as addition | `c557c0a4a437d430d1452d84835665ed7fbc9cbf` | 160 | 149 |
| Repeated sizes assigned through a local temporary | `aaa50c6d0a4fd3d056a846b0128af231f721c365` | 160 | 149 |
| Inverted initial direction test | `7f0d4896f1c54b1158ab68933be88d78ad10f37d` | 160 | 149 |

The retained source hash (including its explanatory comment) is
`f5c21d78e8d823d326a9a022787008e0d244f83c`.
Its isolated GCC 2.9 probe places a 158-byte function at `0x6C88`, followed
by two alignment bytes: 160 candidate bytes versus 176 expected. The report
still marks it `DIFF` (142 differing bytes); it is not integrated or counted
as recovered code. No shared build, symbol manifest, or region ledger was
changed.
