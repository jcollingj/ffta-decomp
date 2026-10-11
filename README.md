# ffta-decomp

Matching decompilation of **Final Fantasy Tactics Advance** (USA, `AFXE`).

This repository starts with a matching reconstruction and incrementally recovers editable data and code. Existing community research is linked in [the roadmap](docs/ROADMAP.md).

## Status

- ✅ Byte-identical rebuild of the ROM from source (`make` verifies SHA-1)
- ✅ First carved region: cartridge header (`asm/header.s`)
- ✅ Second carved region: title screen palette (`data/title_palette.s`, ROM `0x9581D8`)
- ✅ Headless emulator harness with CLI control (`tools/emu/fftactl`)
- ✅ Bounded ability-name edit (`make mod-ability-name`): First Aid → Field Aid
- ✅ Paused screenshots, named checkpoints, and durable baseline backup tools
- ✅ Post-tutorial battery save and savestate, backed up and freshly reloaded
- ✅ First Aid → Field Aid verified in-game after cold-loading the baseline save
- ⏳ Main-game save and cross-build runtime proof: see [work log](docs/WORK_LOG.md)

The matching rebuild still copies almost all bytes from the base ROM. It is not a completed decompilation. See [the roadmap](docs/ROADMAP.md) and [ability text notes](docs/ABILITY_TEXT.md).

## Setup

Requires: `arm-none-eabi-binutils` (Homebrew), Python 3, and a legally
dumped FFTA (USA) ROM placed at `baserom.gba`
(SHA-1 `4ac05441f4de70a4ec3dd932116346c61b8783d9`). The ROM is never committed.

```sh
make            # build build/ffta.gba and verify it matches baserom
make COMPARE=0  # build without the match check (for modified ROMs)
```

## How decompilation proceeds

The ROM starts as giant `.incbin` chunks of `baserom.gba`. To decompile
something, split the incbin around it and replace that region with real
source (asm or C) that assembles to the identical bytes. `make` proves it.
`data/title_palette.s` is the model example.

## Emulator control (SkyEmu headless HTTP server)

`tools/emu/fftactl` wraps SkyEmu's HTTP control API so the game can be
driven entirely from the CLI (or by an AI agent):

```sh
tools/emu/fftactl start            # boot build/ffta.gba headless on :8080
tools/emu/fftactl step 900         # advance 900 frames
tools/emu/fftactl press Start 10   # hold Start for 10 frames
tools/emu/fftactl screen out.png   # screenshot
tools/emu/fftactl read 05000000 32 # read emulated memory (hex addr)
tools/emu/fftactl write 02000000=ff
tools/emu/fftactl state save 0     # save/load state slots
tools/emu/fftactl checkpoint save checkpoints/working/example.state.png
tools/emu/fftactl status           # inspect pause state and loaded ROM
tools/emu/fftactl stop
```

Requires the SkyEmu **dev build** in `/Applications/SkyEmu.app`
(the v5 release crashes in headless mode on macOS; build from
https://github.com/skylersaleh/SkyEmu).

Screenshots leave emulation paused. Use `step` or `press` to advance, and `run` only for deliberate live playback. `press A 5` advances five held frames plus one released frame; `mash A 31` advances exactly 31 frames. The controller accepts `SKYEMU` and `FFTA_PORT` environment overrides and stops only processes it launched and recorded.

The installed SkyEmu revision advances one frame when restoring a checkpoint. Account for that offset in exact replay comparisons. Controller invocations are recorded in `logs/actions.jsonl`.

The browser viewer runs with `cd viewer && bun dev`, at http://localhost:3000. A headless emulator does not show a normal Dock window. The viewer observes the screen and logs; it does not drive the game.

## Saves and the first gameplay mod

Once the game has been saved through its own menu:

```sh
tools/emu/baseline capture main-game --note "World map after opening; saved through System menu"
tools/emu/baseline verify checkpoints/baselines/main-game
tools/emu/baseline prepare checkpoints/baselines/main-game checkpoints/runs/baseline-load
make mod-ability-name
tools/emu/baseline prepare checkpoints/baselines/main-game checkpoints/runs/ability-name --rom build/mods/ability-name/ffta.gba
```

These commands are a workflow example; consult the work log for the names of actually verified artifacts. Capture creates a second copy under `backups/baselines/`, marks files read-only, and includes checksums. Both directories are local and ignored by Git. Keep an additional copy off this machine for protection against disk loss. Runtime save validation requires cold-booting a working copy and using the game's Load menu. Do not point the emulator at the protected baseline.

Run `python3 -m unittest discover -s tests -v` for controller and ROM-edit regression checks. Set `LUNA_MODEL` and `CODEX_BIN` to configure the CLI Luna launcher; model access depends on the authenticated account. Only one controller should drive the active emulator at a time.

## Demo mod

```sh
python3 tools/mods/swap_title_palette.py   # blue-shift the title screen
make COMPARE=0
tools/emu/fftactl start
git checkout data/title_palette.s          # restore matching source
```

## Full heal mod and battle recordings

`make mod-field-aid` builds Field Aid with full HP recovery and native Regen. See [ability editing](docs/ABILITY_TEXT.md) for exact scope and verified battle results.

Use `tools/emu/clip start NAME`, play through `fftactl`, then `tools/emu/clip stop` to save a silent MP4. The viewer's Watch menu plays completed clips. See [recording instructions](docs/RECORDING.md) for the verified Field Aid demo and reusable checkpoints.

The viewer's [Decomp guide](http://localhost:3000/guide) explains ROM/save compatibility, the current decomp process, tooling, ability expansion, and the proposed law-removal work. The page source is `viewer/src/Guide.tsx`; it distinguishes locally verified capabilities from future work and external research.

Run `make progress` for audited decomp coverage. The machine-readable snapshot is `docs/decomp-progress.json`, driven by the reviewed `config/decomp_regions.json` ledger. Source-first AI improvements, map editing, and the eventual map editor are planned in [the roadmap](docs/ROADMAP.md).

Matching C is now wired into the ROM build using a pinned local `pret/agbcc` compiler (`make compiler`). Use `make compare` for function byte/layout checks, `tools/decomp/probe.py` for isolated nonmatching candidates, and `tools/decomp/xrefs.py` for cached reference searches. Commands and limitations are documented in [the roadmap](docs/ROADMAP.md#active-goal-and-fast-recovery-workflow).
