# Content consumer host composition harness

`tests/test_content_consumer_composition_c.py::ContentConsumerCompositionTests.test_full_real_chain_composition` builds and runs
`tests/c/content_consumer_composition_harness.c` against the production sources as
separate translation units. It composes `ConsumeContentRecord` with
`GetContentConsumerContext`, `FillContentConsumerEntries`,
`ResolveContentConsumerSelection`, `FindContentEntryWithUnitFlag12`, the three
context-list count wrappers, and `ReadCurrentContentIndex`. Their real visitor,
predicate, payload adapter, append callback, list accessor, and unit-field helper
dependencies are linked as well. The fixture supplies only backing globals and
records; it does not replace any function in this chain.

The flag-`0x10` branch uses the real +0x14 list at lengths 0, 1, and 13. These
cases cover empty traversal, a null nested unit, the full caller array, record
indices 0 and 1, and qualification by bit 0 of unit halfword `half18[8]` (field
27). The 13-entry image also sets bit 12 on other units, so the assertion checks
the consumer's bit-0 rule separately from the finder's bit-12 rule.
The tested length is capped at the actual output capacity because the recovered
fill routine appends every list payload without a bound.

The clear-flag cases pass distinct unit selectors through the real identity
lookup. The +0x10 chain starts with an entry whose nested unit is null, then
reaches a valid bit-12 unit, exercising the real null-safe unit helper. One list
has duplicate selector identities with values `0xffff` and
`0x4321`; the expected first output is zero, proving that the first match was
selected and incremented with 16-bit wrap. Its +0x10 list has a nonmatching unit
followed by a bit-12 match, and its +0x14/+0x18 lengths are 3 and 2, giving an
expected second output of 6. The no-bit-12 case expects 5. This exercises the
finder's bit-12 predicate separately from the flag branch's bit-0 count.

Each call copies a fully initialized fixture and global state into expected
images, changes only the two expected output words, then compares the complete
fixture and backing records/index state after the call. It also checks both
global context pointers. Thus list links, node payloads, entries, nested units,
opaque bytes, record bytes, and index bytes must remain unchanged.

The older controlled consumer harness remains in place for its stub-specific
edge cases. This composition harness validates integration behavior on the host;
it does not add recovered ROM bytes or change decompilation credit.
