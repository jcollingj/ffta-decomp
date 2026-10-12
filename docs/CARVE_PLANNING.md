# Read-only copied-region carve planning

`tools/decomp/carve_plan.py` prepares a reviewable plan for replacing a
bounded interval inside one copied ROM region. It reads `config/decomp_regions.json`,
the copied source's `.incbin`, and the `OBJS` list. It does not edit any files,
read or write ROM bytes, run a build, or claim that proposed C matches the
original game.

Offsets use an inclusive start and exclusive end. Both boundaries must be
four-byte aligned. The interval must fit wholly inside exactly one copied
ledger region and overlap no recovered region. The complete ledger must also
pass the existing progress validator, so unrelated gaps, overlaps, or invalid
region entries reject the plan. The existing copied source's
`.incbin` offset and length must agree exactly with the ledger. The tool rejects
unrecognized incbin syntax, unsupported OBJS syntax, unresolved source/object
relationships, duplicate or colliding proposed names, and any empty interval.
Source paths must be canonical repository-relative paths that resolve inside
the root. New C sources use `src/NAME.c`; copied remainders use `asm/NAME.s` or
`data/NAME.s`.

Example machine-readable plan:

```sh
python3 tools/decomp/carve_plan.py \
  --start 0x40 --end 0x50 \
  --source src/carve_example.c --json
```

Optional `--prefix` and `--suffix` values are relative source paths such as
`asm/rom_section_prefix.s`. If a remainder exists and no name is supplied, the
tool proposes a deterministic name based on the old copied source. Prefix,
replacement, and suffix objects appear in that order at the old object's
position. Only nonempty remainders are emitted, with their actual incbin offset
and length. Whole-region, start-edge, and end-edge carves therefore never
create zero-length incbins.

The plan lists files that a later integration would create, edit, and retire.
It includes `ld/rom.ld` among required edits for linker layout assertions; the
old copied source is explicitly marked for retirement after its replacement
and remainder sources are integrated. The plan also gives the full proposed
OBJS sequence and the unrelated objects preserved in their original order.
These are proposals only. Integration still requires reviewing the source,
manifest, build list, and linker assertions, then independently verifying the
unchanged matching build. This planner does not satisfy that gate or prove
that the new source is matching C.

Run the planner against a stable working tree after acquiring the integration
lease. Concurrent source or build-list changes can make source/object resolution
fail, and any plan becomes stale once those build files change; reread and
regenerate the plan before editing.

Prompt interpretation: `--prefix` and `--suffix` name source files (relative
paths including their extension), rather than bare stems or object files.
Four-byte alignment is a required planning constraint for both boundaries.

Focused tests use disposable temporary repository layouts:

```sh
python3 -m unittest tests.test_carve_plan -v
```
