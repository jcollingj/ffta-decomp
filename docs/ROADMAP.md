# FFTA editing and gameplay roadmap

The goal is to make Final Fantasy Tactics Advance USA (AFXE) editable and to verify changes in the running game. Preserve a byte-matching baseline while progressively replacing ROM regions with documented source and editable data.

## Chosen direction: source-first decompilation

Recover original engine code and data into maintainable source, preserving an unchanged matching build before modifying each recovered subsystem. Existing byte-patch experiments proved the testing loop; future abilities and law changes should be implemented through recovered source rather than accumulating unrelated ROM hooks. Handwritten assembly may remain where appropriate for low-level code, but copied bytes and disassembly do not count as decompiled C.

The testing loop is sufficient to begin this work, not a complete automated regression suite. Next establish a ROM/RAM and function inventory, determine a matching compiler configuration from representative functions, add function-level comparison and layout assertions, and recover the ability lookup/effect path as the first complete subsystem. Promote successful gameplay routes into bounded replay scenarios with explicit state assertions.

Track recovered code, editable data, and opaque copied regions separately. Do not use total ROM size as the denominator for C decompilation progress before code/data classification. A reliable completion forecast requires measured throughput across several representative functions and subsystems.

## Product goals and implementation order

The source-first decomp must support substantially better enemy AI, editable battle maps and other game content, new abilities, optional law-free gameplay, and eventually a visual map editor. Preserve original behavior as the reference; implement improvements as explicit source configurations after recovering the relevant subsystem. Do not make speculative calendar estimates the delivery plan: use measured milestones and working artifacts.

1. **Measure and map.** Maintain the audited region ledger and progress snapshot; recover code/data boundaries, functions, symbols, RAM structures, and compiler fingerprints. Establish matching C builds and function-level diffs.
2. **Recover the ability path.** Replace the relevant copied definitions and lookup/effect routines with matching source, then reproduce Field Aid as a source-level variant. Add deterministic scenario fixtures and state assertions.
3. **Recover tactical foundations and AI.** Map units, battle state, navigation, targeting, action legality, damage/status forecasts, RNG, candidate generation, and action scoring. Reproduce original AI before changing it. Evaluate improvements on reproducible scenarios: legal actions, useful healing/status use, target selection, terrain/positioning, objective play, and bounded turn computation. Compare outcomes across multiple controlled seeds; win rate alone is insufficient and balance changes must be explicit.
4. **Recover map and mission formats.** Document terrain tiles, elevation, collision/traversability, graphics/palettes, deployment and enemy positions, map references, mission objectives, and event triggers as discovered. Build lossless extraction/import and validate an unchanged round trip before modifying a map. Check placement legality, reachability, height behavior, and event compatibility in game.
5. **Build the map editor on the recovered format.** Start with a read-only renderer/inspector, then editable tiles/heights/deployment, undo/redo, validation, and export through the same build pipeline. Add mission/event editing only after its schema and interpreter are recovered. Keep a portable structured map format as the source of truth, not an editor-only opaque file.
6. **Recover and configure laws.** Implement law-free behavior in recovered source, covering enforcement, UI/cards/judges, story interactions, and death/save semantics.
7. **Complete engine and content recovery.** Continue through saves, jobs/items, UI/text, scripts, rendering/animation, audio, and remaining systems. Eliminate unknown copied game-code regions and provide documented editable data and asset pipelines.

These workstreams share dependencies; map format research can inform AI navigation while ability recovery establishes battle structures. A subsystem is not finished merely because its source compiles: unchanged builds must match and intentional changes need behavioral evidence.

## Progress measurement

Run `make progress` to validate the matching ROM and regenerate `docs/decomp-progress.json` from `config/decomp_regions.json`. The ledger covers every ROM byte with no gaps or overlaps and is checked against `.incbin` declarations in the matching build. It remains a reviewed inventory, not an automatic function detector.

Initial baseline (2026-10-11): 0 matching C functions, 0 recovered assembly-code bytes, 543 authored data bytes, 1 generated checksum byte, and 16,776,672 copied bytes. Thus 544 of 16,777,216 ROM bytes (0.003242%) are source-backed/generated. The 32-byte header figure comprises 31 authored bytes plus the generated checksum; the palette contributes 512 bytes. Total function count and code-byte denominator are unknown. Host-side tools, patch capability, game testing, and copied data are not counted as decompiled engine code. Preserve this initial measurement when reporting later changes.

## Starting state

The October 2026 audit forced a rebuild and verified SHA-1 `4ac05441f4de70a4ec3dd932116346c61b8783d9`. Only 32 header bytes and a 512-byte title palette are explicitly authored; the remaining bytes come from the supplied base ROM. No game functions have been decompiled. The viewer builds and SkyEmu supports screenshots, inputs, memory reads, and checkpoint files.

Historical gameplay reached dialogue after the first Ivalice battle, but no battery save or post-tutorial checkpoint survived. A fresh boot confirmed this. Screenshots are evidence, not backups. The title checkpoint is the only original recovery artifact found.

## Immediate acceptance test

1. Let Luna complete the opening and reach normal main-game play.
2. Save using the game's Save menu and capture a separate emulator checkpoint.
3. Copy both outside `build/` into an immutable baseline directory, alongside ROM and emulator identities and checksums.
4. Restart the emulator and load a working copy of the battery save. Confirm the same party/progression.
5. Rename one accessible ability in a separately built modified ROM.
6. Cold-boot that ROM with a copy of the baseline battery save, open the ability menu, and record the changed label.
7. Rebuild the matching ROM and verify that the original baseline still matches. Preserve before/after screenshots and commands.

Use a battery save for cross-build tests. Emulator states contain transient RAM and execution state and may retain cached text or be incompatible with changed code/layout. Never overwrite the baseline during testing.

## Foundation

- Pin toolchain and emulator versions; add a dependency/ROM doctor command.
- Separate matching and modified outputs; keep source ROM read-only.
- Validate source ROM size/hash before carving or patching.
- Add ROM region boundaries, size assertions, and linker maps. Fixed-layout edits must never silently shift following regions.
- Preserve authored Luna/viewer sources in version control; ignore local sessions, logs, extracted assets, ROMs, and saves.
- Keep local save backups outside the directory removed by `make clean`.

## Reliable gameplay testing

### Verification gates as recovery grows

Run `make -j4 check` before accepting recovered code or tooling changes. It runs the matching build, every inventoried function's byte/address/size comparison, the Python regression suite, and the ROM provenance audit. The audit writes `build/check-progress.json` without changing the published progress snapshot and checks the full ROM identity even if `COMPARE=0` was passed. Use isolated candidate probes during iteration; keep the complete gate at integration points so recovery stays fast.

Test verification tools with deliberately invalid disposable inputs: wrong reference ROMs, corrupted/truncated outputs, missing or moved symbols, wrong symbol sizes, and dishonest coverage claims must fail. Fixtures must never mutate the real base ROM or protected saves. Exact original bytes are the primary acceptance oracle for unchanged recovered code; host-compiled behavioral tests alone cannot prove GBA compiler, packing, or hardware behavior.

As data formats are recovered, add lossless round-trip fixtures and malformed-input/bounds checks. As behavior changes, add bounded emulator scenarios with known ROM/fixture hashes, controlled inputs and seeds where available, and explicit post-action state assertions. For maps, require unchanged binary round trips before edits, then terrain/deployment validation and in-game movement/event checks. For the editor, cover import/export, undo/redo, and invalid edits. Add these tests with the corresponding implementation; they are planned gates, not currently completed coverage.

- Observations never resume emulation. Live playback is explicit.
- Validate buttons/frame limits; release inputs even after failures.
- Verify loaded ROM identity, own emulator processes, and avoid broad process termination.
- Expose status, named checkpoint save/load, and exact-duration actions.
- Record actions, ROM hashes, emulator revision, screenshots, and bounded scenario results.
- Account for SkyEmu's one-frame advance on state restore in replay assertions. The audit's mismatch was explained by this behavior; a follow-up isolated test matched 31 uninterrupted frames against load plus 30 explicit frames. Broader scenarios still need coverage.
- Luna explores visually; successful routes become replayable scenarios. Add verified RAM observations for active unit, HP, menu state, cursor, and battle outcome as structures are recovered.
- Assert success from game evidence rather than agent narration alone. Cover boot, load/save, menus, movement, ability use, battle completion, and map transitions.

## Editable content and engine recovery

First recover a useful narrow data format and prove unchanged extraction/reimport reproduces identical bytes. Expand to items, jobs, abilities, text, graphics, maps, missions, and event scripts. Document pointers, string control codes, compression, encoders, range limits, and relocation rules.

Build a ROM/RAM map, symbol inventory, ARM/Thumb annotations, function boundaries, cross-references, and confidence/evidence records. Establish a pinned C compilation workflow and matching comparison before replacing recovered functions. Keep decompiled-code coverage separate from editable-data coverage and copied-ROM coverage.

Grow engine editing incrementally: recover a function, verify behavior, replace its original region, then add supported expansion/relocation where needed. Full engine recovery is a long-term project; useful content editing can land earlier.

## Agent interface

The intended interface is `observe`, `act`, `checkpoint`, `restore`, `run_scenario`, and `report`. Each scenario declares a starting fixture, objective, action/time budget, assertions, and a stop reason. Only one gameplay controller owns a session at a time. Memory edits and cheats are excluded from normal baseline creation.

## References

- [SkyEmu HTTP API](https://raw.githubusercontent.com/skylersaleh/SkyEmu/dev/docs/HTTP_CONTROL_SERVER.md)
- [mGBA scripting API](https://mgba.io/docs/scripting.html), a possible alternate verification backend.
- [FFTA Decode research](https://github.com/kttalley/ffta-decode-public), a functional research effort rather than a byte-matching rebuild; verify findings locally and check licensing before reuse.

## Completion tracking

See `docs/WORK_LOG.md` for verified changes, artifacts, commands, and outstanding limitations. Do not mark a milestone complete based only on a tool returning `ok`.

The initial save-and-rename milestone is complete as of 2026-10-11. Both cold loads and the renamed ability display passed; see `WORK_LOG.md` for evidence. Autonomous Luna gameplay still needs the reliability improvements above.

## Active goal and fast recovery workflow

The active goal is full source-first decompilation, with a completely accounted original ROM and byte-identical rebuilt output, followed by a demonstrated hand-authored map change and an editor based on its recovered format. A matching build that still copies unknown game code is not completion. Copied code, decoded data, matching C, and necessary low-level assembly remain separately reported. Map modification/editor acceptance remains pending until the prerequisite recovery is complete.

A working C path is now integrated. `make compiler` builds pinned `pret/agbcc` locally under ignored `tools/.deps/`; the revision and observed toolchain are recorded in `config/toolchain.json`. The configuration matches the currently recovered functions, not yet every compilation unit. Upstream's compiler build is deliberately serial because its generated-header prerequisites race under parallel make. Game C object builds can run incrementally and in parallel.

Useful commands:

```sh
make -j4                          # incremental matching ROM build
make compare                      # compare all recovered function bytes and symbol layouts
python3 tools/decomp/compare.py GetAbilityProperty --disassemble
python3 tools/decomp/probe.py src/ability.c --start 0xccd50 --end 0xcce60
python3 tools/decomp/xrefs.py 0xccd50
make progress
```

`compare.py` builds incrementally unless `--no-build` is used and writes `build/function-comparison.json`; optional original/built disassembly goes under `build/compare/`. `probe.py` compiles a candidate at the original address without modifying the ROM build and writes assembly/ELF/bytes under `build/probes/`. It inherits external symbol addresses and function modes from the existing build ELF; unknown references must first be documented. Absolute function symbols must preserve Thumb type, or the linker may create interworking veneers and silently change call code/layout.

`xrefs.py` indexes candidate Thumb BL instructions and word-aligned ROM pointers in a cached SQLite database. It is not a disassembler or proof of function boundaries: data can resemble either encoding. Verify hits against control flow before adding functions to the ledger. Initial index generation took about 1.06 seconds; repeated queries about 0.01 seconds. No-change build/comparison took about 0.03–0.25 seconds; isolated C compilation/comparison about 0.12–0.33 seconds on this machine. Measurements exclude one-time compiler compilation and are observations, not latency guarantees.

Incremental C compilation uses compiler-generated transitive header dependencies (`build/*.d`); changing a subsystem header does not rebuild unrelated C objects. Recovered functions include the complete packed job-property/inheritance reader and low-level save-image byte/flag accessors. See `docs/ABILITY_TEXT.md` for the verified layouts and remaining semantic uncertainty.

### Candidate comparison diagnostics

`probe.py` now compares known functions at their actual candidate symbol locations as well as checking the complete original-address byte interval. Each function reports size, address shift, differing bytes, and a strict MATCH/DIFF status. Identical bytes at the wrong address remain DIFF. Optional `--disassemble` saves original and candidate instruction listings at comparable ROM offsets beneath the probe directory. JSON reports include both raw block equality and the stricter `accepted` result; a layout failure cannot produce a successful exit.

Known matching references come from the recovery ledger. `config/decomp_candidates.json` separately records reviewed research bounds for unfinished routines. It is diagnostic input only: its entries do not contribute recovered bytes/functions, do not complete the function inventory, and are never automatically promoted by a successful probe. Integration still requires source review, linker assertions, the full-ROM comparison, and `make check`.

## Parallel recovery pilot

At the user's request, run three `gpt-6-luna` coding agents with the parent as reviewer/integrator. Assign separate candidate source files and tests; agents may read shared sources but must not edit each other's files. Give each task original ROM bounds, a specific isolated probe command, and explicit completion evidence. The current probe directory is keyed by source filename stem, so concurrent tasks must use distinct stems.

Agents do not change the main build, region/function manifests, linker symbols, shared headers, published progress, or Git history. They return their best source, focused test results, exact byte/address/size differences, and remaining uncertainties. The parent independently reviews semantics and tests, reruns isolated probes after agents stop editing, integrates only exact matches, then runs `make -j4 check progress` and the viewer build before committing/pushing. Research and useful tests may be preserved without admitting a candidate to recovered coverage. Never equate three agents with a promised threefold recovery rate; track accepted output and review cost from the pilot.

Gameplay remains a separate single-controller activity. Coding agents do not operate the emulator or touch protected baselines.

## Continuous-learning Luna experiment: 100 sessions

The user set a target of 100 total Luna sub-agent sessions, counting the initial three. `config/luna_sessions.json` is the authoritative register; one distinct spawned agent counts once, while feedback/revision turns in that same agent do not add sessions. Report started and reviewed counts separately. Never fill the quota with empty tasks or count planned sessions as executed. The broader full-decomp/map objective remains unchanged.

The parent is accountable for assignment quality. Before each task, specify owned files, original byte/function bounds when known, verified dependencies, acceptance evidence, and the relevant lessons from `config/luna_lessons.json`. Require a brief evidence-first approach, then implementation and verification. Prefer substantive recovery/data/tool tasks that can be independently accepted; reserve a minority of sessions for fixing measured workflow bottlenecks.

For each review correction: identify the error and original evidence, ask what the parent omitted or made unclear, send feedback to the same agent, require the agent to revise and add a distinguishing test, then record a reusable lesson or tool check. Do not default to silently fixing the agent's code. Integration edits, necessary emergency fixes, and escalations are recorded separately. Another agent can independently audit high-risk behavior without sharing ownership.

The aspiration is for Luna to perform 90% of the work over time, not to manufacture a line-count statistic. Record measured parent review/integration minutes, agent elapsed minutes, revision rounds, first-review acceptance, parent corrective edits, and accepted matching code/data. Concurrent elapsed time is not person-effort or cost; unavailable token/cost figures stay unknown. Compare batches of ten reviewed sessions using accepted output and intervention, and revise task selection/prompts based on evidence. Keep first-pass tests, iteration history, and final byte comparisons distinct. The parent independently runs the integration gates and publishes only verified progress.
