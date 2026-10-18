# Unit equipment values 14 and 15

The source candidates in `src/nonmatching/unit_clamped_values.c` model two
adjacent Thumb routines from ROM offsets `0xCA580..0xCA60F`. The first
function occupies 66 bytes at `0xCA580..0xCA5C1`; the following two-byte
alignment halfword at `0xCA5C2..0xCA5C3` brings the first region to 68 bytes.
The second function occupies 76 bytes at `0xCA5C4..0xCA60F`. Together the
complete block is 144 bytes.

Both routines inspect the five equipment halfwords at unit offsets
`0x2A`, `0x2C`, `0x2E`, `0x30`, and `0x32`. Empty IDs are skipped; repeated
IDs cause repeated property reads. `ReadUnitEquipmentValue14` snapshots the
low 16 bits at `0xD2`, interprets that snapshot as signed, adds the u32 sum of
selector 14 values, narrows the total to signed 16-bit, then clamps negative
results to zero. `ReadUnitEquipmentValue15` snapshots the low 16 bits returned
by unit selector 32 before reading items, interprets that snapshot as signed,
adds the u32 sum of selector 15 values, narrows the total to signed 16-bit,
and clamps to `[0,100]`. The item sum and its addition to the base use 32-bit
unsigned wrapping before the signed16 narrowing.

The C declarations use `signed int` results. The observed call sites consume
the returned register as an integer value, sometimes storing it to a halfword
or applying later arithmetic; the numerical clamp ranges alone do not establish
a narrower C return type.

The host harness compiles production source separately from controlled getter
doubles. Its fixed vectors cover empty slots, all five positions, zero and
repeated IDs, callback mutation of the captured base and a later equipment
slot, exact property selectors/call order, 32-bit accumulation overflow,
signed16 wrap before each clamp, and complete guarded unit images after every
invocation. Host layout assertions check the actual field offsets.

Matching comparison was attempted against immutable
`build/verified-339-content-ref.elf` (SHA-256
`1cb594bdd8655baecca519a6272d57e187b872bb57b40fbe01002189fafb0d29`) with:

```sh
python3 tools/decomp/probe.py src/nonmatching/unit_clamped_values.c \
  --start 0xca580 --end 0xca610 \
  --reference-elf build/verified-339-content-ref.elf --disassemble
```

The retained natural C candidate is not byte-matching: it emits 160 bytes
(72-byte first symbol, followed by an 86-byte second symbol and 2 trailing
alignment bytes) against the 144-byte original block. The first original
function is 66 bytes plus its 2-byte region padding; the candidate's first
symbol is 72 bytes. The second candidate starts at `0xCA5C8`, four bytes after
the original second function at `0xCA5C4`. Symbol-aligned comparisons find 57
differences across the first function's 66 overlapping bytes, plus 6 bytes of
candidate excess; the second function has 69 differences across 76 overlapping
bytes, plus 10 bytes of candidate excess. Comparing the first 144 bytes of the
raw blocks gives 132 differing bytes; the candidate has 16 extra bytes, so the
probe reports 148 total differences including the length delta. Full block
equality, including the original alignment halfword, is false. A comparison of
the first 68-byte region (function plus padding) would count 59 differences;
that is a region metric, not a function metric.

The retained production source SHA-256 is
`6efd0b0c7c79e6136bb8801c5fc35c5639f979fd07bd4463403767ba028e6ed4`. During
bounded research, the typed equipment-pointer iteration shape emitted 124
bytes, explicit byte-pointer indexing emitted 124 bytes at `-O2` and `-O1`
(292 bytes at `-O0`), and the retained `i != 5` typed-field loop emitted 160
bytes at `-O2`. The final candidate's comparison JSON and disassembly remain
under `build/probes/unit_clamped_values/`; this is research evidence only and
earns no recovered-byte or function credit. No volatile or register-allocation
hints are used.
