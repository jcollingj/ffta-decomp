# Recording battle clips

With SkyEmu paused and a ROM loaded, use a unique clip name:

```sh
tools/emu/clip start my-ability-test
# Play using tools/emu/fftactl press, step, and mash.
tools/emu/clip status
tools/emu/clip stop
```

FFmpeg must be installed. Recordings sample the emulator framebuffer every two emulated frames and encode a silent 30 fps MP4 at 960×640. Agent thinking time is omitted. This captures the game, not the desktop. Files and ROM hashes are stored in `logs/clips/NAME/`; generated media stays out of Git. The browser viewer at http://localhost:3000 offers a Watch selector and Download MP4 link.

Use `FFTA_PORT` consistently for the recorder and controller when targeting an isolated emulator. While recording, drive only through `fftactl`: direct HTTP steps or another controller bypass capture. Live running, ROM loads, resets, state restores, and emulator start/stop are blocked by the controller during recording. Stop recording before any of these operations. If capture fails, `clip abort` retains raw evidence and releases the recording lock; aborted or incomplete clips are excluded from the viewer. Clip names cannot overwrite earlier recordings.

## Field Aid example

- Highlight: `logs/clips/field-aid-demo/clip.mp4` (68 seconds, labeled cuts).
- Full recording: `logs/clips/field-aid-full-heal-regen/clip.mp4` (234.2 seconds).
- The highlight's `recording.json` records source hash, exact excerpt times, captions, and output hash. It shows the full heal, deliberate subsequent damage, and later turns with Regen. Waiting and unrelated menus are cut; gameplay within excerpts is unchanged.
- Pre-cast state: `checkpoints/working/field_aid_before_cast.state.png`.
- Verified state: `checkpoints/working/field_aid_regen_verified.state.png`.
- Use the matching modified ROM in `checkpoints/working/field-aid-battle/ffta.gba` for those states. That directory's battery save is the accepted mission on the world map, not an in-battle save.

The earlier `field-aid-effect-debug` clip used a cross-ROM state for diagnosis and is not the cold-load acceptance evidence.

## Playback controls

In the viewer, Right/Left changes clip speed by 0.25×, bounded between 0.25× and 4×. Buttons provide the same controls and 1×, 1.5×, 2×, 3×, and 4× presets. Up jumps forward 15 seconds; Down jumps back 15 seconds. Jumps use the video timeline regardless of playback speed and stop at the clip boundaries. Keyboard shortcuts work while a clip is selected, except when editing a field or operating a dropdown. Selecting a different clip resets speed to 1×.
