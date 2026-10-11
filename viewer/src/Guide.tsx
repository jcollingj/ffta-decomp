import progress from "../../docs/decomp-progress.json";

const engineSource = 'https://github.com/LeonarthCG/FFTA_Engine_Hacks/tree/14ad10060ee21474c56272fd94bcc21d5a92264c';

export function Guide() {
  return <div className="guide-shell">
    <header className="hdr">
      <h1>FFTA Tactical Ops</h1>
      <nav className="app-nav" aria-label="Main navigation"><a href="/">Game & clips</a><a href="/guide" aria-current="page">Decomp guide</a></nav>
    </header>
    <main className="guide-main">
      <div className="guide-intro">
        <p className="eyebrow">PROJECT HANDBOOK · UPDATED OCTOBER 11, 2026</p>
        <h2>From a ROM to an editable game</h2>
        <p>We can already rebuild the original game, make guarded ability edits, reuse a post-tutorial save, and record battle proof. Full engine decompilation is still ahead.</p>
        <div className="guide-badges"><span>Verified: save → mod → battle</span><span>Engine functions decompiled: {progress.matching_c_functions}</span></div>
      </div>
      <nav className="guide-toc" aria-label="Guide sections">
        <a href="#progress">Progress</a><a href="#plan">AI & map plan</a><a href="#saves">ROMs & saves</a><a href="#process">Decomp process</a><a href="#abilities">New abilities</a><a href="#laws">Engine & laws</a><a href="#tools">Our tools</a><a href="#constraints">Constraints</a>
      </nav>

      <section id="progress" className="guide-section">
        <p className="eyebrow">MEASURED BASELINE</p><h2>Progress we can audit</h2>
        <p>Run <code>make progress</code> to refresh these numbers. Last matching-build audit: <time>{progress.verified_utc.slice(0, 10)}</time>. This is a build-time snapshot, not a live scan.</p>
        <div className="guide-cards">
          <article><span className="status-tag">RECOVERED CODE</span><h3>{progress.matching_c_functions} matching C functions</h3><p>Total game functions: unknown. The code inventory is not complete, so an engine-completion percentage is not available.</p></article>
          <article><span className="status-tag">ROM PROVENANCE</span><h3>{progress.source_or_generated_percent.toFixed(6)}% source-backed</h3><p>{progress.source_or_generated_bytes.toLocaleString()} of {progress.rom_bytes.toLocaleString()} bytes are authored or generated. This measures ROM coverage, not engine completion.</p></article>
          <article><span className="status-tag planned">STILL COPIED</span><h3>{progress.bytes_by_kind.copied.toLocaleString()} bytes</h3><p>Copied from the base ROM. Includes code and data whose full boundaries are not yet classified.</p></article>
        </div>
        <p>The initial 544 bytes comprise 31 header bytes, 1 generated checksum, and a 512-byte palette. Recovered assembly-code bytes: {progress.bytes_by_kind.recovered_asm}. Host-side tooling, patched bytes, and gameplay recordings do not count as decompiled game code.</p>
        <p>The reviewed ledger in <code>config/decomp_regions.json</code> covers the whole ROM without gaps or overlaps. The auditor checks the original build hash and copied-region declarations against build sources. Function status and region classifications require evidence and review; they are not inferred from a passing boot.</p>
      </section>

      <section id="plan" className="guide-section">
        <p className="eyebrow">THE SOURCE-FIRST PLAN</p><h2>Better tactical AI. Editable maps. A map editor.</h2>
        <p>These are core goals alongside new abilities and optional law-free gameplay. Recover each original subsystem, prove the unchanged build, then implement improvements in source. Measure progress by working milestones.</p>
        <ol>
          <li><strong>Build the decomp foundation.</strong> Audited coverage, ROM/RAM and function maps, compiler identification, matching C builds, and function-level diffs.</li>
          <li><strong>Recover abilities end to end.</strong> Data definitions, lookup, effect dispatch, healing/status routines, and source-based Field Aid. Turn the battle proof into a repeatable scenario.</li>
          <li><strong>Recover and improve tactical AI.</strong> Understand navigation, legal actions, targeting, forecasts, RNG, candidate generation, and scoring. Preserve vanilla AI first, then test better positioning, healing, target choice, and objective play on controlled scenarios and multiple seeds. Measure decision quality and turn cost as well as outcomes.</li>
          <li><strong>Recover map and mission formats.</strong> Round-trip tiles, terrain, elevation, collision, graphics, deployment, objectives, and event references as their structures are discovered. Prove one edited map in battle before building the editor.</li>
          <li><strong>Build the map editor.</strong> Read-only renderer/inspector first; then tile, height, and deployment editing with undo/redo, validation, and export through the normal build. Mission/event editing follows schema and interpreter recovery.</li>
          <li><strong>Recover laws and finish the remaining systems.</strong> Implement deliberate law-free behavior in source and continue through saves, jobs/items, UI, scripts, graphics/animation, audio, and other engine code.</li>
        </ol>
        <p>Map reachability, valid placements, terrain height behavior, and script compatibility need validation. The editor should use a documented structured format shared with the importer, so maps remain editable outside the UI. The detailed living plan is <code>docs/ROADMAP.md</code>.</p>
      </section>

      <section id="saves" className="guide-section">
        <p className="eyebrow">01 / PERSISTENCE</p><h2>Changing the game does not replace your progress</h2>
        <div className="save-flow" aria-label="The ROM supplies game definitions and the battery save supplies progress. Booting and loading combines them into a running session.">
          <div><strong>ROM · .gba</strong><p>Code, ability definitions, text, maps, graphics, scripts</p></div><span aria-hidden="true">+</span>
          <div><strong>Battery save · .sav</strong><p>Persistent party and progression data written by the game</p></div><span aria-hidden="true">→</span>
          <div><strong>Running session</strong><p>Boot the chosen ROM, then use the game's Load menu</p></div>
        </div>
        <p>The save is not a copy of the entire game. When the engine looks up an existing ability in the ROM, it uses that ROM's definition. Our same-ID Field Aid edit therefore changes what the party can do without replaying the tutorial. Exact save fields and all lookup paths have not yet been mapped.</p>
        <div className="guide-table"><table><caption>Which changes can reuse a save?</caption><thead><tr><th>Change</th><th>Expected save relationship</th><th>Our evidence</th></tr></thead><tbody>
          <tr><td>Rename an existing ability</td><td>Existing references resolve to new text; IDs and save structure stay intact.</td><td><strong>Verified.</strong> Same post-tutorial save cold-loaded into original and renamed builds.</td></tr>
          <tr><td>Change an existing ability's effects</td><td>Same ability identity, different behavior when used.</td><td><strong>Verified for Field Aid.</strong> 42/59 → 59/59 HP, with Regen observed in battle.</td></tr>
          <tr><td>Change starting stats, rewards, or event scripts</td><td>Previously stored stats, collected rewards, or completed event flags may remain. Changes may only affect future events or new games.</td><td>General compatibility risk; not yet tested here.</td></tr>
          <tr><td>Reorder IDs, expand learned-ability storage, change save structures</td><td>Old values may mean different things or no longer fit. Requires a validated migration or a new save.</td><td>No migration tooling or compatibility guarantee yet.</td></tr>
        </tbody></table></div>
        <h3>A savestate is a different kind of artifact</h3>
        <p>Our <code>.state.png</code> files are SkyEmu checkpoints containing execution state and memory; ordinary screenshot PNGs are only pictures. A checkpoint resumes a captured execution point and can restore cached data. Use it for fast recovery with the matching ROM and emulator. A changed build may need different memory, pointers, or code, so loading an old checkpoint is not proof that an old battery save works.</p>
        <h3>The repeatable cross-build test</h3>
        <ol><li>Keep the original baseline immutable. Prepare a new working directory with a copy of its battery save and the chosen ROM.</li><li>Stop the owned emulator, boot the modified ROM, then choose Saved Game → Load → slot 1. Do not load a checkpoint for this acceptance test.</li><li>Verify party/progress, exercise the edit, save in-game to the working copy, and cold-load again to test persistence.</li><li>Keep ROM hashes, save identity, emulator revision, screenshots, and assertions with the result. Preserve a separate untouched save for going back.</li></ol>
        <p>Our baseline is <code>post-tutorial-2026-10-11</code>, duplicated under <code>checkpoints/baselines/</code> and <code>backups/baselines/</code>. The battery file is 65,536 bytes. The working layout uses <code>ffta.gba</code> beside <code>ffta.sav</code>; check SkyEmu's reported save path rather than assuming any renamed ROM automatically finds the right save.</p>
        <p>Field Aid's accepted test cold-loaded the original baseline and then entered Herb Picking. Its mission-accepted working save and battle checkpoints are separate artifacts. We have not yet verified every possible mod combination or long-term playthrough.</p>
      </section>

      <section id="process" className="guide-section">
        <p className="eyebrow">02 / THE PROCESS</p><h2>Matching bytes first, editable systems next</h2>
        <p>Decompilation recovers readable source from machine code. A byte-matching build proves our rebuild reproduces the original ROM; it does not prove that all of that ROM has been understood. The initial build authored only the header and title palette. We now compile {progress.matching_c_functions} recovered C functions into their original addresses. Most of the ROM is still copied with <code>.incbin</code>; the measured coverage above records what remains.</p>
        <ol className="process-steps">
          <li><strong>Establish the reference.</strong> Hash the exact USA ROM and reproduce it with <code>make</code>. Keep matching and modified outputs separate.</li>
          <li><strong>Map one region.</strong> Recover record layouts, pointers, encodings, function boundaries, and ARM/Thumb code mode. Record evidence and remaining uncertainty.</li>
          <li><strong>Make it editable.</strong> Replace copied data with a round-trip extractor/importer, or recover a function in source. Unchanged input must reproduce the original bytes.</li>
          <li><strong>Build a bounded change.</strong> Guard original bytes and layout, write a separate ROM, and inspect the change manifest. New space and relocated pointers need explicit ownership.</li>
          <li><strong>Prove behavior.</strong> Cold-load a battery-save copy, run a scenario, check actual outcomes, record evidence, and verify save/reload. Turn successful routes into repeatable regression scenarios.</li>
        </ol>
        <p>Our chosen direction is source-first decompilation: recover and verify the original subsystem before changing its behavior in source. The existing byte-patch demos established the testing loop; they will not be the main editing architecture. A pinned Thumb C compiler, incremental builds, function comparison, isolated candidate probes, and cached reference searches are now in place. We still need a complete function map, broader compiler validation, relocation support, general data editors, and automated scenarios.</p>
      </section>

      <section id="abilities" className="guide-section">
        <p className="eyebrow">03 / CONTENT</p><h2>Can we add new abilities? Yes—with different levels of work.</h2>
        <div className="guide-cards">
          <article><span className="status-tag">VERIFIED</span><h3>Change an existing ability</h3><p>Field Aid reuses First Aid's ID 76 and its existing menu/learning connections. We changed three name bytes and two effect bytes. Full heal, cleansing, and Regen use routines already in the engine. This does not add an ability slot.</p></article>
          <article><span className="status-tag planned">NEXT CAPABILITY</span><h3>Introduce a distinct ability</h3><p>Audit a candidate unused slot or extend the table, then wire the new ID into a job command list and a learning source. Add name/help text, targeting, cost, effects, animation, and AI behavior. An apparently empty record is not automatically safe to reuse.</p></article>
          <article><span className="status-tag planned">ENGINE WORK</span><h3>Create a new mechanic</h3><p>If existing effects cannot express the behavior, recover effect dispatch and implement a new routine or hook. Handle target validity, battle preview, execution, AI evaluation, and status interactions consistently.</p></article>
        </div>
        <p>The current map has 346 ability records of 28 bytes at ROM offset <code>0x55187C</code>. Our patcher validates this table and only supports the demonstrated edit; it is not an “add ability” editor. Record count is the current layout, not proof of a hard engine maximum or proof that extra records will work.</p>
        <p>Expansion requires tracing all references, loop bounds, ID widths, terminators, menu limits, and learned-ability/AP storage. If learning storage cannot represent another ability, preserving old saves may require a migration. A new entry is not accessible until jobs, equipment or another learning path expose it.</p>
        <p><strong>Recommended first addition:</strong> a distinct test ability made from existing effects, using an audited slot and one explicit learning path. Prove menu display, learning/mastery, casting, enemy use, and save → restart → load before expanding the system.</p>
      </section>

      <section id="laws" className="guide-section">
        <p className="eyebrow">04 / ENGINE CHANGES</p><h2>Disabling laws is feasible. It is not implemented here yet.</h2>
        <p>Major systems can be changed by replacing recovered routines or inserting guarded hooks. The practical constraints are understanding their dependencies, preserving calling conventions and memory layout, and fitting code within the GBA's resources. The scale of the change determines the reverse-engineering and testing effort.</p>
        <p>There is concrete precedent: <a href={engineSource}>LeonarthCG's FFTA Engine Hacks</a> includes judge/law removal. Its <a href={`${engineSource.replace('/tree/', '/blob/')}/ROM%20Buildfile.event`}>build configuration</a> describes a <code>noJudges</code> option and says it does not enable permadeath. That is an upstream claim, not a verified feature of our build. We have not imported it, audited all dependencies, or established permission to reuse its code.</p>
        <h3>Proposed implementation path</h3>
        <ol><li><strong>Start with enforcement disabled.</strong> Trace violation checks and penalties, preserving battle flow and existing save fields. Make the variant a separate build option. This is the first milestone, not complete removal.</li><li><strong>Remove the remaining system behavior.</strong> Audit judge presence, law generation/display, cards, rewards/JP interactions, prison/release, and scripted battles. Decide deliberately what replaces each player-facing feature.</li><li><strong>Verify unusual cases.</strong> Normal battles, Jagds and KO/death behavior, story battles, existing penalized units, AI turns, victory/escape, and save/reload all need coverage. Removing a judge must not accidentally alter death rules or block story progression.</li></ol>
        <p>The goal can be a fully law-free game. Hiding the law UI alone does not achieve that. We should first document the engine checks and choose the intended judge/story behavior, then test a minimal enforcement change before removing the surrounding systems.</p>
      </section>

      <section id="tools" className="guide-section">
        <p className="eyebrow">05 / THE CURRENT TOOLKIT</p><h2>What we can use today</h2>
        <div className="guide-table"><table><thead><tr><th>Tool</th><th>Use</th><th>Current boundary</th></tr></thead><tbody>
          <tr><td><code>make</code> / <code>make check</code> / <code>make progress</code></td><td>Rebuild and compare the original ROM; run the Python regression suite.</td><td>Matching scaffold, not a complete decomp. Tests do not establish full game correctness.</td></tr>
          <tr><td><code>tools/decomp/compare.py</code><br/><code>tools/decomp/probe.py</code><br/><code>tools/decomp/xrefs.py</code></td><td>Fast C build/comparison, isolated candidates at original addresses, and cached ROM reference searches.</td><td>Compiler configuration is verified on the recovered functions only. Reference hits are candidates until their callers are decoded.</td></tr>
          <tr><td><code>make mod-ability-name</code><br/><code>make mod-field-aid</code></td><td>Guarded edits, separate output ROMs, hashes and changed-byte manifests.</td><td>Exact USA input only; names must retain encoded length. No generic relocation or arbitrary effect editor.</td></tr>
          <tr><td><code>tools/emu/baseline</code></td><td>Capture, verify, and prepare baseline copies, with read-only duplicate backups.</td><td>Checksums verify artifacts; runtime boot/load still needs testing. Same-disk copies are not off-machine backups.</td></tr>
          <tr><td><code>tools/emu/fftactl</code></td><td>Owned SkyEmu process, buttons, exact frame steps, screenshots and checkpoints.</td><td>One controller per session. Screenshots stay paused; state restore advances one frame in the installed SkyEmu revision.</td></tr>
          <tr><td><code>tools/emu/clip</code></td><td>Start/stop silent emulator-frame MP4 capture with ROM identity.</td><td>FFmpeg required. All steps must pass through fftactl during recording; raw HTTP bypasses capture.</td></tr>
          <tr><td>Luna gameplay agent</td><td>Visual gameplay through the harness model <code>gpt-6-luna</code>, with local briefing and checkpoints.</td><td>No separate Decisions API exposed. Parent interventions were needed; not yet a reliable unattended test runner.</td></tr>
          <tr><td>This Bun/React viewer</td><td>Live screen, logs, clip playback/download, speed presets, keyboard controls, and this guide.</td><td>Observes the emulator; it does not drive gameplay. Clip arrows change playback only.</td></tr>
        </tbody></table></div>
        <h3>Example: build a variant and prepare its save</h3>
        <pre><code>{`make mod-field-aid\ntools/emu/baseline prepare \\\n  checkpoints/baselines/post-tutorial-2026-10-11 \\\n  checkpoints/runs/my-field-aid-test \\\n  --rom build/mods/field-aid/ffta.gba\n# Choose a fresh destination for every run.\ntools/emu/fftactl stop\ntools/emu/fftactl start checkpoints/runs/my-field-aid-test/ffta.gba\n# In the game: Saved Game → Load → slot 1`}</code></pre>
        <p>Recording: <code>tools/emu/clip start unique-name</code>, play through <code>fftactl</code>, then <code>tools/emu/clip stop</code>. The viewer offers Left/Right ±0.25× and Up/Down ±15 video seconds, independent of playback speed.</p>
      </section>

      <section id="constraints" className="guide-section">
        <p className="eyebrow">06 / GUARDRAILS & NEXT STEPS</p><h2>What constrains an edit?</h2>
        <ul><li><strong>Version and layout:</strong> offsets target the verified 16 MiB USA ROM, SHA-1 <code>4ac05441f4de70a4ec3dd932116346c61b8783d9</code>. Other regions or prepatched ROMs are not interchangeable.</li><li><strong>Space and references:</strong> longer strings, bigger tables, or new code require verified free space or relocation plus all affected pointers. Do not shift adjacent bytes blindly.</li><li><strong>Encoding and execution:</strong> text uses custom character/control codes; assets may be compressed. Engine edits must respect ARM/Thumb mode, registers, stack, branches, timing, and RAM limits.</li><li><strong>Save semantics:</strong> a file loading successfully is not proof every stored ID or flag still means the same thing. Keep old/new save families separate until compatibility is established.</li><li><strong>Shared definitions:</strong> our Field Aid edit affects every user of First Aid, potentially including enemies. It remains free and self-targeted, and its original help text does not mention Regen.</li><li><strong>Evidence:</strong> use actual post-action HP/status and save/reload, not a targeting forecast or agent narration. Current recorded proof covers one battle scenario, not all status combinations.</li><li><strong>Artifacts:</strong> keep ROMs, saves, recordings, and extracted assets local and out of Git. Track authoring code, manifests/specifications, and documentation; audit third-party licensing before copying code.</li></ul>
        <p>Next foundations: recover the save/ability-learning layout, audit candidate ability slots, build an ability data editor with round-trip tests, map law enforcement entry points, and turn the recorded battle into a bounded replay scenario.</p>
        <h3>Evidence and further reading</h3>
        <p>Repository records: <code>docs/ROADMAP.md</code>, <code>docs/WORK_LOG.md</code>, <code>docs/ABILITY_TEXT.md</code>, <code>docs/RECORDING.md</code>, and <code>AGENTS.md</code>. This page is maintained in <code>viewer/src/Guide.tsx</code>.</p>
        <p><a href="/clips/field-aid-demo">Watch the Field Aid proof</a> · <a href="/clips/field-aid-full-heal-regen">Full recording</a> · <a href="https://github.com/TojiKitten/FFTA-randomizer/blob/master/src/main/ffta/FFTAData.ts">Ability/text mapping research</a> · <a href={engineSource}>Engine-change precedent (pinned revision)</a></p>
      </section>
    </main>
  </div>;
}
