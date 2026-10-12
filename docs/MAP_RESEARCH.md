# Battle map format reconnaissance

Status: reconnaissance only. No battle-map record, geometry schema, map loader, or map round trip is recovered here. This note records the strongest local evidence found and gives the next bounded investigation step without assigning map semantics to generic content records.

## Verified local evidence

| Anchor | Direct observation | What it establishes | What it does not establish |
|---|---|---|---|
| `0x0800A018` (`ReadCurrentContentIndex`, Thumb entry `0x0800A019`) | Original instructions load `0x020020D0`, add `0xA0`, load a halfword, and return. The function body is `[0x0800A018, 0x0800A020)` (8 instruction bytes); the literal at `0x0800A020` is the RAM base. Thus the smallest code-plus-literal recovery block is 12 bytes, `[0x0800A018, 0x0800A024)`. `asm/external_symbols.s` supplies the provisional external name. | A copied helper returns the halfword at RAM `0x02002170`. Several call sites use it as an index. | Its gameplay role, valid index range, and relation to a battle map.
| `0x0854CD54` (`gContentRecords`) | `ld/rom.ld:8` provides the address alias. `src/content_values.c` uses a 40-byte C record stride, a pointer at offset 28, and reads bytes 0/1 through that pointer when game flag `0x253` is clear. This agrees with the original code and prior disassembly recorded in `docs/WORK_LOG.md:134-140`. | There is a content-record access path with an observed stride and one pointer-valued field. | The table extent, pointer target format, field meanings, or any map association. The 40-byte shape is only partly known; the source explicitly leaves the rest unknown.
| `[0x0802BA40, 0x0802BAF6)` | This bounded function invokes `0x0800A018` at `0x0802BA60`, computes `index * 40`, and adds `0x0854CD54`. It then reads a byte at record offset `+32` (`0x0802BAAC`). Two padding bytes occupy `[0x0802BAF6, 0x0802BAF8)` before the next entry at `0x0802BAF8`. The pointer loads at `0x0802BA8E/90` traverse a separate helper-filled stack list and are not fields in the indexed record. | This is a confirmed content-record consumer with an observed byte at offset 32. | It is not identified as a map loader; the byte's meaning and any map association are unknown.
| `0x080B8834`, `0x080B890C`, `0x08123672` | Disassembly confirms calls to `0x0800A018`. The first two also compute the same 40-byte record stride and base. | Multiple subsystems share the current-content index and content records. | These call sites are not independently classified as map/mission code.

The source-first roadmap requires terrain, height, collision/traversability, graphics/palettes, deployment, map references, objectives, and triggers to be documented as discovered; none of those map-specific structures is present in the current source/data inventory. A whole-word search for map and geometry terms in code, data, and the function/region inventories found no matches. The substring `mission` alone matches the ordinary word “admission” in `config/decomp_candidates.json`, so the search uses word boundaries:

```sh
rg -n -i '\b(battle[ -]?map|terrain|elevation|tile|deployment|mission|collision|traversab|height)\b' \
  src include data asm tools/decomp config/decomp_regions.json config/decomp_candidates.json
```

That search result is a repository-name/source inventory check, not evidence that those concepts are absent from the ROM. `tools/decomp/xrefs.py 0x0800A018 --limit 100` reports 20 Thumb-BL candidates; direct disassembly confirmed only the inspected callers above. `tools/decomp/xrefs.py 0x0854CD54 --limit 100` reports 27 pointer candidates. As the tool documents, both lists can contain data lookalikes and are leads, not proven references. Its successful run also validated the local base ROM against the expected SHA-1 `4ac05441f4de70a4ec3dd932116346c61b8783d9`.

No external map labels or online schema claims were used. The evidence is from repository source/metadata and direct disassembly of the local ROM; no data payload was copied or counted as recovered.

## Best next bounded work

The smallest sound code target is the 12-byte `ReadCurrentContentIndex` code-plus-literal block `[0x0800A018, 0x0800A024)` (8 instruction bytes plus its 4-byte literal), currently represented only as an external alias. Recovering it as C would verify the helper, but would not by itself advance map-format recovery. The nearest data-tracing target is the bounded content-record consumer `[0x0802BA40, 0x0802BAF6)`; it reads a byte at indexed-record offset `+32`, but its semantic role is unknown, so do not name it a map routine yet.

For the map workstream, the next gate should be **map-entrypoint identification**: inspect the unclassified callers and pointer consumers, confirm a path from an observed battle/map selection to a specific ROM pointer and loader, and establish the loader's code bounds before decoding bytes. Only then choose one referenced payload and document its exact source interval, pointer/relocation behavior, and any decompression step. The first format milestone should be an unchanged extraction/re-import whose output is byte-identical to that interval. Geometry editing should wait until tile dimensions, per-cell field boundaries, elevation representation, and collision/traversability use are independently confirmed. No image or geometry milestone is supportable from the current evidence.

## Reproduction commands

These commands are read-only with respect to the source ROM; the xref helper may refresh its ignored SQLite cache under `build/research/`.

```sh
python3 tools/decomp/xrefs.py 0x0800A018 --limit 100
python3 tools/decomp/xrefs.py 0x0854CD54 --limit 100
arm-none-eabi-objdump -D -b binary -marm -Mforce-thumb \
  --adjust-vma=0x08000000 --start-address=0x0800A018 --stop-address=0x0800A040 baserom.gba
arm-none-eabi-objdump -D -b binary -marm -Mforce-thumb \
  --adjust-vma=0x08000000 --start-address=0x0802BA40 --stop-address=0x0802BB00 baserom.gba
```
