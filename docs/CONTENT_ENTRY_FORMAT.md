# Content source entry inspection format

This tool losslessly inspects source entries reached through one explicitly
selected 40-byte content record. Evidence establishes the record stride, three
byte transfers, and five equipment IDs consumed by one observed routine. It
does not identify the larger table's bounds or describe a map.

## Observed layout

The USA ROM SHA-1 is `4ac05441f4de70a4ec3dd932116346c61b8783d9` and its size is
16 MiB. At `0xCA744`, the original loader reads a halfword count at record
offset `+0` (`0xCA758`), reads a pointer at `+4` (`0xCA752`), and advances each
source entry by `0x30` bytes (`0xCA768–0xCA76A`). This establishes the 48-byte
source stride; it does not establish global table bounds.

In the routine beginning at `0xCA268`, raw Thumb halfwords include
`0x3108` at `0xCA272` (add 8 to the source argument) and `0x220A` at
`0xCA276` (set the copy length to 10). The following call sequence uses the
copy routine pointer literal `0x0836D4BC` at `0xCA2E4`; this is the
`gCopyRecordMemory` slot used by the existing source/linker declarations. The
ten copied bytes therefore occupy source offsets `+8..+17`. In the later loop,
halfword loads at `0xCA2BC` and `0xCA2C4` use the incrementing entry pointer,
and the loop advances that pointer by two bytes at `0xCA2D4`; this path consumes
the five copied values as halfword equipment IDs. Zero IDs are retained as
values; the routine still passes them through its item-processing calls.

The copied initializer at `0xC9644` separately retains the source argument in
`r8` and destination argument in `r7`. Instructions `0xC96A0–0xC96B0` form and
save destination addresses `destination+0xF6`, `+0xF7`, and `+0xF8`.
Instructions `0xC9ABE–0xC9AD0` load source bytes `+5`, `+6`, and `+7` and store
them through those saved addresses in the same order. Values `0..255` are
retained as raw bytes; consumer-specific signed or unsigned interpretation is
outside this format.

The target record is exactly 40 bytes. Its known count and pointer are at `+0`
and `+4`; the remaining record bytes are not interpreted by this tool.
Extraction rejects a selected record unless all 40 bytes fit in the ROM. For
nonempty payloads, the 32-bit target pointer is translated from GBA ROM address
space by subtracting `0x08000000`; the entire `count*48` span must fit the
16 MiB ROM. No inferred adjacent-record boundary is used.

For a zero count, a null pointer is accepted and represented with a null file
offset and no entries. A zero count with a ROM-backed pointer is also accepted
when its translated offset is within the ROM or exactly at its exclusive end;
no bytes are dereferenced. These are safe empty spans, not claims about whether
any particular game record is valid. Non-ROM pointers remain outside this
inspection tool's accepted input.

## JSON versions

Version 1 remains accepted by rebuild with its original exact entry shape:

```json
{
  "opaque_prefix_hex": "0142000100",
  "byte5": 7,
  "byte6": 4,
  "byte7": 0,
  "opaque_suffix_hex": "00000000000000000000000003000000000000000000000000000000000000000000008000000000"
}
```

That shape represents five opaque prefix bytes, bytes 5/6/7, and forty opaque
suffix bytes. It retains its historical interpretation exactly; no bytes in its
suffix are reinterpreted as equipment IDs.

New extraction emits version 2. Its entry shape is:

```json
{
  "opaque_prefix_hex": "0142000100",
  "byte5": 7,
  "byte6": 4,
  "byte7": 0,
  "equipment_ids": [264, 10, 0, 65535, 20],
  "opaque_suffix_hex": "000000000000000000000000000000000000000000000000000000000000"
}
```

The five equipment IDs are unsigned 16-bit integers encoded little-endian at
source bytes `8..17`; the example's first ID 264 is bytes `08 01`. The remaining
30 bytes (`18..47`) stay opaque. Prefix and suffix hex strings must have exactly
five and thirty bytes respectively. IDs must be a list of exactly five strict
integers in `0..65535`; booleans and floats are rejected. Entry objects reject
unknown or missing keys. Version 1 documents must use only the v1 shape, and
version 2 documents only the v2 shape; mixed forms are rejected.

Both documents use the same top-level shape:

```json
{
  "format": "ffta-content-source-entries",
  "version": 2,
  "rom_sha1": "4ac05441f4de70a4ec3dd932116346c61b8783d9",
  "record_offset_hex": "0x0054CD7C",
  "record_count": 8,
  "source_pointer_hex": "0x0852A4D0",
  "source_file_offset_hex": "0x0052A4D0",
  "entries": []
}
```

The example abbreviates `entries`; real documents contain exactly
`record_count` entries. Rebuild validates the fixed format/version/ROM hash,
metadata consistency, exact counts and widths, and the 16 MiB record/payload
bounds. It emits only the concatenated source entries, exactly 48 bytes each,
with no ROM header or neighboring data. Direct `encode_entry` and
`decode_entry` APIs default to version 2 and accept an explicit strict version
argument (`1` or `2`) for version-specific shapes.

```sh
python3 tools/decomp/content_entries.py extract baserom.gba \
  --record-offset 0x54CD7C \
  --output build/content-entries/record-54cd7c.json

python3 tools/decomp/content_entries.py rebuild \
  build/content-entries/record-54cd7c.json \
  --output build/content-entries/record-54cd7c.bin
```

Extraction requires the complete ROM to have the fixed USA SHA-1. Output paths
must be under `build/content-entries/`; writes use a temporary file and atomic
no-clobber publication. Existing files and symlinks are refused. The tool
never writes the ROM input. Generated JSON and payload files stay under the
ignored `build/` tree.

## Independently checked sample spans

Both samples below were read directly from `baserom.gba` after verifying its
full SHA-1. The hashes cover only the stated payload spans. They are test
evidence, not evidence that any unlisted table records are valid.

| Record file offset | Count | Target pointer | Payload file offset | Payload bytes | Payload SHA-1 | Payload SHA-256 |
| --- | ---: | --- | --- | ---: | --- | --- |
| `0x54CD7C` (index 1) | 8 | `0x0852A4D0` | `0x52A4D0` | 384 | `fb31b5dc6170a16a0e2d956705543aa9e1ef8305` | `6c73e7c84cfe0e2e2af6b98b019a4020ea8923fad45fede4e8a5b8aa3370bf04` |
| `0x54CDA4` (index 2) | 9 | `0x0852A650` | `0x52A650` | 432 | `6e5f57db1042576f56486b499042d0e50d2c6ab3` | `60a9f3e7efd79227dbeb9e59d37515b2213ad518e9c8babece89adcc19afc645` |

The first record at `0x54CD54` is deliberately not used as a sample. These two
records do not prove a global table extent. Opaque byte round-tripping is
format tooling only and earns no decompiled-byte credit.
