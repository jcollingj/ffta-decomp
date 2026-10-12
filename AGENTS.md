# Working on FFTA

Read `docs/ROADMAP.md` and `docs/WORK_LOG.md` for scope and verified progress.

- Preserve `baserom.gba`. The expected USA ROM SHA-1 is `4ac05441f4de70a4ec3dd932116346c61b8783d9`.
- Keep matching builds and modified ROMs separate. `make` checks the matching output; `make mod-ability-name` builds the first bounded data edit.
- Do not delete or overwrite `checkpoints/baselines/` or `backups/baselines/`. Use `tools/emu/baseline prepare` to create writable working copies.
- A screenshot is not a savestate. Save milestone checkpoints and use the game's Save menu for battery saves. Verify a cold boot and Load before claiming a reusable baseline.
- Delegate ordinary gameplay to Luna using `.claude/skills/luna-driver/SKILL.md`. Only one agent/controller drives a session at a time. Parent agents can inspect screenshots and debug tools without sending competing inputs.
- Screenshots stay paused. Use explicit frame steps; SkyEmu restore itself advances one frame. Never silently enable live running for test scenarios.
- Record assertions, screenshots, hashes, and restart/load results in `docs/WORK_LOG.md`; do not infer success from narration or HTTP `ok` alone.
- Run `python3 -m unittest discover -s tests -v` after controller/mod changes, and verify matching output with `make`. Follow `viewer/CLAUDE.md` when editing the Bun viewer.
- Keep generated ROMs, saves, checkpoints, screenshots, and private session logs out of Git. Authoring tools and documentation belong in the repository.
- Record clips with `tools/emu/clip start NAME` and `stop`; during capture, all frame advances must go through `fftactl`, never raw HTTP. See `docs/RECORDING.md`.
- An existing in-game save slot defaults to No on overwrite. Confirm Yes and verify the save before reporting persistence.

- Project direction: source-first decompilation. Recover original code/data and verify the unchanged matching build before introducing feature changes in that subsystem. Existing byte-patch demos remain testing evidence; do not grow them into the main implementation architecture.
- Product goals include better tactical AI, editable maps/content, and a visual map editor. Follow the source-first milestones in `docs/ROADMAP.md`; recover and round-trip formats before building authoring interfaces.
- Update `config/decomp_regions.json` when replacing copied ROM regions, and run `make progress`. Report code recovery separately from ROM/data provenance; unknown code/function totals must remain unknown.

- Luna learning experiment: target 100 unique spawned Luna coding sessions, counting the first three. Follow the continuous-learning section of `docs/ROADMAP.md`; read `config/luna_lessons.json` before assigning or implementing a task. Track actual sessions in `config/luna_sessions.json`. Review errors are feedback to the same agent for revision and a reusable system improvement, not a reason to silently take over. Report unknown effort/cost metrics as unknown and count only independently verified recovery.
