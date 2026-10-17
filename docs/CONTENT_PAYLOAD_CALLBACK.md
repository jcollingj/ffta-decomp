# Content payload callback adapter

`ContentPayloadCallbackAdapter` recovers the bridge at ROM offset `0x9ABE8`.
The original USA ROM hash is `4ac05441f4de70a4ec3dd932116346c61b8783d9`.
The inspected 24-byte interval has SHA-256
`5ad8a4fabc18d64da541a3302cae642e74b23363331b058743c06d2fdac95cf0`;
the symbol occupies 22 bytes and the final two bytes are zero padding.

The Thumb instructions save `r4` and `lr`, preserve the second argument in
`r4`, call `GetListOwnerWord0` at `0xC7B74`, load pair word 0 into `r1` and
word 1 into `r2`, then call the `bx r2` bridge at `0x14224C`. That bridge
branches through `r2`, so the pair fields are the callback context and
callback function, respectively. The callback result remains in `r0` and is
returned unchanged. The source uses the established `ListPayload` native
pointer-width word, and performs no callback function-pointer cast.

The focused host harness compiles the adapter, `GetListOwnerWord0`,
`VisitLinkedNodes`, and `ContentAppendCallback` as separate translation units.
It checks empty and three-node traversal against whole fixed owner, node,
pair, append-state, and guarded output images; confirms the three payload
addresses by identity and in list order; and checks the adapter's forwarded
context mutation plus return values `0`, `1`, `7`, and `-1` using fixed
callback-visible state images. Through the real visitor it checks that `7`
continues, zero stops before the third node, and `-1` and `1` continue through
all three nodes. The harness is run with:

```sh
python3 -m unittest discover -s tests -p 'test_content_payload_callback_c.py' -v
```

The isolated agbcc probe against immutable reference ELF
`build/verified-286-content-ref.elf` (SHA-256
`463c56de5fa6e3ea3a512b3d319483ee254af625a38b42894083a384f3b917f2`)
matches all 24 bytes at `0x9ABE8..0x9AC00`: 22 symbol bytes plus two zero
padding bytes. The generated adapter symbol is 22 bytes at `0x9ABE8`.

The outer copied caller wrapper is outside this recovery, and no whole content
traversal or subsystem is claimed recovered here. The recovered source is
admitted to the matching build at `src/content_payload_callback.c`;
`make compare` reports the 22-byte symbol at `0x0809ABE8` with zero differing
bytes. Its 24-byte ledger region includes the two required padding bytes.
