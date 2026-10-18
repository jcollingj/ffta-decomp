# Working on FFTA

Read `docs/ROADMAP.md` and `docs/WORK_LOG.md` for scope and verified progress.

## Persistence and the meaning of “continue”

- The standing objective is 100% source-first decompilation with a fully accounted, byte-identical original build, followed by a verified hand-authored map change and a working editor built on recovered formats. Opaque copied game code does not satisfy completion.
- Every “continue,” “keep going,” or equivalent request means resume sustained work toward that entire objective. It does not mean complete one function, batch, commit, test run, or experiment and stop.
- Treat passing gates, commits/pushes, and milestones as checkpoints. Report them in commentary, record durable progress, then immediately select and execute the next useful recovery task. Do not require another “continue” at a checkpoint.
- Stop voluntarily only when the full objective is verified complete, the user explicitly pauses/cancels/redirects the work, or a hard blocker prevents all useful authorized progress. A difficult nonmatch, failed test, unknown format, exhausted hypothesis budget on one routine, or waiting worker is not by itself a hard blocker: debug, build tools, investigate dependencies, or advance another independent task.
- A hard blocker means required user information, access, or an unavailable external dependency is indispensable and no independent work can proceed. Explain the exact blocker, evidence, attempted remedies, and smallest action needed to unblock it. Do not invent blockers or silently lower acceptance criteria to finish.
- Keep source recovery, verification, documentation, and authorized checkpoint publication moving together. Preserve tests, byte comparison, original ROM and baseline safeguards; persistence never authorizes bypassing them.
- Before unavoidable context/runtime limits, write a precise handoff with the verified checkpoint, current changes, active owners, next executable tasks and outstanding failures. Resume from that state when execution continues. Repository instructions cannot override platform shutdowns, limits, or a paused harness goal; disclose such constraints accurately instead of promising unattended execution.
- The 100-session Luna experiment is a completed measurement milestone, not a stopping condition for the project. Keep its historical metrics separate from subsequent recovery.

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

- Parallel verification: run focused tests on owned stable files during iteration. Before full test discovery or `make check`, the parent obtains a write-freeze acknowledgement from every active source/test writer. Release the freeze after the gate. Agent-run uncoordinated full discovery is not acceptance evidence.
- Host C recovery tests compile production sources as separate translation units. Use independent fixed expected whole-state images, including node arrays and callback-visible state; do not use a reimplementation of the candidate as the sole oracle.
