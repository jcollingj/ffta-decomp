# Content unit-pool setup

`LoadAndPrepareContentUnitPool` is the matching natural-C recovery for ROM
`0x96E18..0x96E78` (96 bytes). Its behavior is documented from the original
Thumb instructions. The function and region are now in the matching build and
recovery inventory; `ResolveOrCreateContentUnitEntry` at Thumb address
`0x08092441` remains a typed alias for copied code and receives no recovery
credit. The loader at `0xCA744` is a separate matching-C dependency.

## Instruction evidence

The original code narrows its first argument to 16 bits, forms a
`ContentRecord` address with a 40-byte target stride, and directly calls
`0xCA744` with that record, `gUnitPoolA` (`0x02002FC4`), and the literal limit
12. It ignores the call's return value.

It then examines 12 records, advancing by `0x108` bytes. The occupancy test is
the byte at pool-record offset `+4`; zero skips the resolver call. For every
nonzero byte, it directly calls `0x92440` with the list/context argument and
the occupied `UnitRecord` pointer. The result is treated as a
`ContentConsumerEntry *`. Its object pointer at entry offset `+0` selects the
destination object: source byte `+0xF6` is stored at that object's `+0xF9`,
then the entry's object pointer is loaded again and source byte `+0xF7` is
stored at that object's `+0xFA`. The destination may differ from the scanned
pool record. There are no null checks. The loop reloads each later slot's
occupancy after prior resolver calls.

The immediate code at `0x92440` forwards the first argument and unit pointer
to `ResolveContentConsumerSelection` at `0x99560`, supporting the
`ListCountContext *` first-argument view. `ResolveOrCreateContentUnitEntry`
is a typed research alias for the copied routine at `0x92440`; its name does
not claim recovered behavior or dependency credit. The first direct call is
to the loader at `0xCA744`; neither call passes a callback-function literal.

## Candidate and evidence

The candidate source uses the established `ContentRecord`, `UnitRecord`,
`UnitFieldView`, `ContentConsumerEntry`, and `ListCountContext` declarations.
The output-byte mapping uses `UnitFieldView.bytesF1[5..6]` for source bytes
`+0xF6..+0xF7` and `[8..9]` for destinations `+0xF9..+0xFA`.

The initial research probe used the immutable reference
`build/verified-300-content-ref.elf`, SHA-256
`b509bd395e699edb5597f9ce97befea53b2d941efdf41a027995daeaffc88751`. A
build-only copy added temporary typed function-symbol aliases for the then
unintegrated dependencies at `0xCA744` and `0x92440`. This was before the
loader's separate integration. That copy hashes to
`87a580fa68666012e561bf4c336613f6169f7ecadc59b588c32bc9e48a15eb8a` and is
kept under ignored `build/probes/`. The candidate was probed with:

```sh
python3 tools/decomp/probe.py src/content_pool_setup.c \
  --start 0x96e18 --end 0x96e78 \
  --reference-elf build/probes/content_pool_setup/reference-aliases.elf \
  --disassemble
```

The probe reports `LoadAndPrepareContentUnitPool: MATCH`, size 96/96, zero
symbol-aligned differences, and a 96-byte block match. The function and region
entries, linker boundary assertions, and typed copied resolver alias are
integrated. The probed binary SHA-256 is
`5a708f0ec871281f71d982ee32a457950a4ebb975c19d55836ac21ff59c6ca2b`.
The candidate source SHA-256 is
`e0705ff188be25f6fdcba6bcefdaa6b1459735c8a3e9ed3a9c79d3c72f2fa7d7`.

## Host checks and limits

`tests/test_content_pool_setup_c.py` compiles the production source as a
separate translation unit. Controlled dependency stubs verify the loader's
first-call ordering and exact record, pool, and limit arguments; tests cover
u16 index truncation with a synthetic four-record array, an empty pool, sparse
slots with occupancy values other than one, all 12 occupied slots, resolver
order, two distinct returned nested objects, both copied bytes, and a resolver
that changes a later slot's occupancy before the loop reaches it. Each case
compares whole pool, object, entry, context, content-record, and second-pool
images. The resolver trace checks all observed call arguments and order.

The focused test passed with `python3 -m unittest
tests.test_content_pool_setup_c -v`; the same harness also passed a host build
with `-fsanitize=address,undefined`. Integration passed `make compare`,
`make check-wrappers`, and `make`; the ROM hash remained
`4ac05441f4de70a4ec3dd932116346c61b8783d9`. Table bounds, caller preconditions, and
whether actual ROM table index zero is valid remain unknown. The synthetic
index-zero case is not evidence about the original table. Null behavior is
not guarded in the original code, and the host tests do not prove the copied
resolver's target behavior. The harness SHA-256 is
`598e8e6c52580bba548419e6cb7f687dbc18a715488e7406b9341a8cca448fde`.
