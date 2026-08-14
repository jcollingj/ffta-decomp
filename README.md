# ffta-decomp

Matching decompilation of **Final Fantasy Tactics Advance** (USA, `AFXE`).

There is no existing public FFTA decomp — this is from scratch.

## Status

- ✅ Byte-identical rebuild of the ROM from source (`make` verifies SHA-1)
- ✅ First carved region: cartridge header (`asm/header.s`)
- ✅ Second carved region: title screen palette (`data/title_palette.s`, ROM `0x9581D8`)
- ✅ Headless emulator harness with CLI control (`tools/emu/fftactl`)

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
tools/emu/fftactl stop
```

Requires the SkyEmu **dev build** in `/Applications/SkyEmu.app`
(the v5 release crashes in headless mode on macOS; build from
https://github.com/skylersaleh/SkyEmu).

## Demo mod

```sh
python3 tools/mods/swap_title_palette.py   # blue-shift the title screen
make COMPARE=0
tools/emu/fftactl start
git checkout data/title_palette.s          # restore matching source
```
