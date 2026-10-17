# Content source entry inspection format

This tool losslessly inspects source entries reached through one explicitly
selected 40-byte content record. The evidence is limited to the source stride
and three byte transfers below. It does not identify the larger table's
bounds, describe a map, or infer the meaning or signedness of any byte.

## Observed layout

The USA ROM SHA-1 is `4ac05441f4de70a4ec3dd932116346c61b8783d9` and its size is
16 MiB. At `0xCA744`, the original loader reads a halfword count at record
offset `+0` (`0xCA758`), reads a pointer at `+4` (`0xCA752`), and advances each
source entry by `0x30` bytes (`0xCA768–0xCA76A`). This establishes the 48-byte
source stride; it does not establish global table bounds.

In the copied initializer at `0xC9644`, the source argument is retained in
`r8` and the destination argument in `r7`. Instructions `0xC96A0–0xC96B0`
form and save destination addresses `destination+0xF6`, `+0xF7`, and `+0xF8`.
Instructions `0xC9ABE–0xC9AD0` load source bytes `+5`, `+6`, and `+7` and store
them through those saved addresses in the same order. The JSON names those
three transferred bytes only by source offset. Other five prefix bytes and
forty suffix bytes remain opaque. Values `0..255` are retained as raw bytes;
consumer-specific signed or unsigned interpretation is outside this format.

The target record is exactly 40 bytes. Its known count and pointer are at
`+0` and `+4`; the remaining record bytes are not interpreted by this tool.
Extraction rejects a selected record unless all 40 bytes fit in the ROM.
For nonempty payloads, the 32-bit target pointer is translated from GBA ROM
address space by subtracting `0x08000000`; the entire `count*48` span must fit
the 16 MiB ROM. No inferred adjacent-record boundary is used.

For a zero count, a null pointer is accepted and represented with a null file
offset and no entries. A zero count with a ROM-backed pointer is also accepted
when its translated offset is within the ROM or exactly at its exclusive end;
no bytes are dereferenced. These are safe empty spans, not claims about whether
any particular game record is valid. Non-ROM pointers remain outside this
inspection tool's accepted input.

## JSON and commands

Version 1 JSON has exactly these top-level fields:

```json
{
  "format": "ffta-content-source-entries",
  "version": 1,
  "rom_sha1": "4ac05441f4de70a4ec3dd932116346c61b8783d9",
  "record_offset_hex": "0x0054CD7C",
  "record_count": 8,
  "source_pointer_hex": "0x0852A4D0",
  "source_file_offset_hex": "0x0052A4D0",
  "entries": [
    {
      "opaque_prefix_hex": "0142000100",
      "byte5": 7,
      "byte6": 4,
      "byte7": 0,
      "opaque_suffix_hex": "00000000000000000000000003000000000000000000000000000000000000000000008000000000"
    }
  ]
}
```

The abbreviated example shows the first entry's field shapes; real documents
contain exactly `record_count` entries. Each entry represents exactly 48
bytes: five opaque prefix bytes, three separate byte values, and forty opaque
suffix bytes. Rebuild validates the fixed format/version/ROM hash, metadata
consistency, exact counts and widths, and the 16 MiB record/payload bounds. It
emits only the concatenated source entries, exactly 48 bytes each, with no ROM
header or neighboring data.

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

On the task machine, separate Python CLI extraction and rebuild commands took
0.0460s and 0.0380s for index 1 (0.0840s combined), and 0.0459s and 0.0378s
for index 2 (0.0837s combined). Extraction includes process startup, one ROM
read, and the full SHA-1 check; rebuild includes process startup and JSON
validation. Each rebuilt payload matched the corresponding length and SHA-1.

The first record at `0x54CD54` is deliberately not used as a sample. These two
records do not prove a global table extent. Opaque byte round-tripping is
format tooling only and earns no decompiled-byte credit.
