# Tree node layout recovery

The tree node occupies 24 bytes on the target. The recovered insertion routine
at `[0xC7AB4, 0xC7B08)` requests 24 bytes from `AllocatorAllocate`, stores its
two data words at offsets 0 and 4, and initializes words at offsets 8, 12, 16,
and 20 to zero. Its comparisons and branches read the node's first data word,
and it links the new node through the child pointers at offsets 12 and 16 while
setting the parent pointer at offset 8. The final word is therefore a distinct
node field rather than allocation padding.

The two callbacks provide its remaining semantics. `ListCallbackSelectClear`
reads word+20, checks bit zero, and conditionally returns the node through its
second argument. `ListCallbackClearFlag0` clears only bit zero in that word.
The native declaration is `unknown00`, `unknown04`, `parent`, `child_a`,
`child_b`, and `flags`; names for the first two words remain unknown.

On a 32-bit target the links are at offsets 8, 12, and 16, `flags` is at 20,
and the structure is 24 bytes. Host tests retain native pointer width: with
64-bit pointers, `flags` is at 32 and the structure is 40 bytes. The header has
conditional compile-time layout checks for these layouts.

Both callbacks now use the exact generic `TreeTraversalCallback(void *,
void *)` signature. Selection receives a `void *` output slot as its context;
when a caller needs a typed result, it can receive into a `void *` temporary
and then convert the value. The clear callback does not inspect its context.
The original clear routine has one machine argument and never reads the second
argument, so its new generic form preserves that behavior for both traversal
calls and direct target callers that leave the second argument unspecified.

The callback functions remain at their original addresses and are checked
against the supplied ROM. Host integration coverage composes the real traversal
with each callback and checks complete node and output state.

The standalone harnesses compile production source files as separate
translation units. This checks that the public function declarations match the
traversal callback type across the actual caller boundary. Fixtures use native
node pointers and full structure images; raw word-array fixtures would test the
old target offsets on a 64-bit host by accident. Existing raw-word list record
accessors are a separate representation and remain outside this tree-node
migration. When a source file contains multiple representations, migrate the
owner-pointer functions individually and leave unrelated raw-record functions
unchanged.
