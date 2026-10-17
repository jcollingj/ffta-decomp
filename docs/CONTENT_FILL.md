# Content consumer list fill wrapper

`FillContentConsumerEntries` recovers the 44-byte routine at ROM interval
`[0x99D08, 0x99D34)` from USA ROM SHA-1
`4ac05441f4de70a4ec3dd932116346c61b8783d9`. The interval SHA-256 is
`3ba89cc9259a7be934eb19d4d4977d036dba7381bbae99fd6fd1e76d4b476db4`.

It initializes a local append state, points its output cursor at caller
storage, creates a context/callback pair for `ContentPayloadCallbackAdapter`,
and visits the list owner at `ListCountContext.unknown14` (offset `0x14`). The
callback literal is `ContentAppendCallback` at `0x9AC21`; the adapter literal
is `ContentPayloadCallbackAdapter` at `0x9ABE9`. The routine returns the
appended count.

The original API has no capacity argument. The caller must provide enough
`void *` slots for every payload in the selected list. The consumer allocates
13 slots, matching the original caller's storage.

The focused host harness compiles the wrapper, visitor, payload getter,
adapter, and append callback as separate translation units. It checks empty,
one-, three-, and thirteen-node lists, ordered payload values, returned
counts, fixed sentinels around and within output storage, and unchanged full
caller/list images. Distinct nonempty lists at context offsets `0x10`, `0x14`,
and `0x18` make the selected owner observable.

The focused test command is:

```sh
python3 -m unittest discover -s tests -p 'test_content_fill_c.py' -v
```

The isolated agbcc probe against immutable reference ELF
`build/verified-289-content-ref.elf` (SHA-256
`98459ab4b3ce0a15ff606bd4a509410da6e06cf29d2f78c756907db7e23040fc`) matches
all 44 bytes at `0x99D08..0x99D34`.
