# Luna coding-session experiment

This report evaluates the 100-session learning experiment recorded in
[`config/luna_sessions.json`](../config/luna_sessions.json). A registry row is
one distinct spawned agent; revision turns stay within that row. `planned`
records are not starts. Results are provisional until every target session has
an independent review recorded.

## How to reproduce the assessment

From the repository root, print the current assessment with:

```sh
python3 tools/decomp/luna_experiment_report.py
```

To refresh only the generated section in this document, run:

```sh
python3 tools/decomp/luna_experiment_report.py --refresh-doc
```

The refresh command requires exactly one pair of markers and replaces text
between them. It leaves surrounding documentation intact. Use
`--json` for machine-readable output or pass a registry path to assess a
synthetic or archived registry.

## Measurement rules and limitations

- Started, reviewed, and planned counts are separate. Completion is true only
  when reviewed count reaches the configured target. Starting session 100 does
  not complete the experiment.
- Accepted matching functions and bytes are counted only from reviewed rows.
  Code, data, and unclassified byte totals remain separate. A reviewed testing
  or tooling task can be useful while receiving zero recovered output credit.
- Numeric sums are the existing registry reporter's totals. That validator
  treats absent integer counters as zero for backward compatibility; the
  accompanying coverage columns disclose how many reviewed records explicitly
  recorded a value versus omitted it. Thus a displayed total of zero is not
  evidence that every record measured zero.
- First-review accepted, not accepted, and unknown are counted separately.
  Any percentage uses only known true/false values and shows its denominator;
  all-unknown batches have no rate. First review includes plan, code, and
  integration feedback, so it is not a clean coding-accuracy measure.
- Parent and agent minutes remain unknown when absent or null. Elapsed time is
  wall time only when both start and finish are recorded. Registration times do
  not establish spawn times, and concurrent elapsed time is not person effort.
  The registry has no reliable cost/token measure, so effort share, speedup,
  and the aspirational 90% agent-effort share cannot be inferred from these
  totals.
- Batches use session ID ranges 1–10, 11–20, and so on. Batch output and
  intervention counts use reviewed records in that range; coverage is shown
  for revisions and parent corrective edits so missing fields are not mistaken
  for observed zeroes.

## Evidence-based prompt improvements

The following are recommendations grounded in recorded review feedback and
lessons, not measured causal claims about productivity:

- For composition work, require a typed-input audit, each loop iteration, each
  conversion/wrapping/clamping stage, and exact external-read traces before
  implementation. Session 99's initial plan omitted an equipment slot,
  included unrepresentable signed-byte values, missed a final cap, and used
  internal-linkage tables across separate translation units; L054 captures
  these corrections.
- For integration tasks, ask for an explicit source-to-symbol map spanning
  declarations, implementation, linker, ledger, tests, and documentation
  before edits. Session 97's draft used a special build path and a renamed
  reader symbol; L055 records the correction to promote the approved candidate
  and preserve consistent exported names.
- Keep review evidence tied to the original instructions, independently
  executed tests/probes, and resulting byte or state comparisons. When review
  finds an error, return it to the same author for revision and a distinguishing
  test, then record a reusable prompt or tool improvement.

## Parent review after all 100 sessions

The experiment completed with 100 distinct sessions independently reviewed. It added 98 matching functions, 4,120 code bytes and 9,592 data bytes, alongside tests, codecs and workflow tools. The experiment completion checkpoint had 343 matching C functions; that is a function inventory, not a full-game completion percentage.

The delegation produced useful verified work, but the intended low-intervention workflow is not demonstrated. First-review acceptance was 27 of 97 known results (three unknown). The final two batches each accepted two of ten on first review. Matching code output rose in later batches, but task sizes, dependencies and task selection changed, so this does not establish faster work. Parent and agent effort measurements are absent for all 100 sessions; cost and the 90% agent-effort aspiration remain unmeasured.

My largest assignment weakness was repeating acceptance requirements without requiring the author to demonstrate how the proposed fixture would detect the specific error. Concrete precode plans helped expose mistakes earlier, but did not eliminate them. I also supplied compiler hypotheses, corrected API/layout assumptions, reviewed integration and published checkpoints. Counting only direct parent code edits would hide that work.

For subsequent work, retain three independent coding slots and serialized integration, but require a compact checked evidence packet: original function and padding bounds; existing dependency declarations; representable inputs with arithmetic stages; neutral prerequisites for each branch; and expected complete live images after every call. Require a source/symbol/path reconciliation before integration and use the existing carve/probe tools instead of hand arithmetic. Record parent review time and actual agent start/finish timestamps prospectively; never backfill guesses. Keep a bounded matching-research budget, preserving tested nonmatches without recovery credit. These are process recommendations grounded in observed failures, not yet verified productivity gains.

## Current generated assessment

<!-- BEGIN GENERATED LUNA ASSESSMENT -->

**Progress:** 100 started, 100 reviewed, 0 planned; target 100. Experiment status: complete.

Only reviewed records contribute accepted-output totals and batch results. Sums below are recorded values; the existing registry reporter treats absent integer counters as zero, so coverage is shown alongside totals.

| Reviewed accepted output | Recorded total |
|---|---:|
| Matching functions | 98 |
| Matching bytes | 13712 |
| Code bytes | 4120 |
| Data bytes | 9592 |
| Unclassified accepted bytes | 0 |
| Reviewed records with nonzero accepted functions or bytes | 52 |

**First review:** 27 accepted, 70 not accepted, 3 unknown; known-only rate 27.8% (27/97 known). This includes plan, code, and integration feedback; it is not a clean coding-accuracy measure.

| Reviewed metric field | Coverage |
|---|---:|
| `accepted_matching_functions` | 100 recorded / 0 missing |
| `accepted_matching_bytes` | 100 recorded / 0 missing |
| `accepted_matching_code_bytes` | 85 recorded / 15 missing |
| `accepted_matching_data_bytes` | 85 recorded / 15 missing |
| `revision_rounds` | 83 recorded / 17 missing |
| `parent_corrective_edits` | 83 recorded / 17 missing |
| `first_review_accepted` | 97 recorded / 3 missing |
| `parent_minutes` | 0 recorded / 100 missing |
| `agent_minutes` | 0 recorded / 100 missing |

| Reviewed intervention metric | Recorded total | Coverage |
|---|---:|---:|
| Revision rounds | 109 | 83 recorded / 17 missing |
| Parent corrective edits | 2 | 83 recorded / 17 missing |

| Effort/timing measure | Recorded sum | Known records | Unknown records |
|---|---:|---:|---:|
| Parent effort minutes | 0 | 0 | 100 |
| Agent effort minutes | 0 | 0 | 100 |
| Elapsed wall minutes | 21.7215 | 4 | 96 |

Elapsed wall time is not person effort; registration timestamps are not spawn times. Concurrent elapsed time cannot establish effort share. Cost/token data are unavailable. No speedup or 90% effort-share claim is supported by this registry.

### Reviewed batches of ten

Accepted output and intervention totals below use reviewed records in each session-ID batch. Metric coverage distinguishes explicit zero from missing data.

| Sessions | Reviewed | Functions | Code bytes | Data bytes | Unclassified bytes | First review T/F/? (known rate) | Revisions | Coverage | Parent edits | Coverage |
|---:|---:|---:|---:|---:|---:|---|---:|---|---:|---|
| 1–10 | 10 | 1 | 52 | 7436 | 0 | 2/5/3 (28.6% (2/7)) | 6 | 7 recorded / 3 missing | 0 | 7 recorded / 3 missing |
| 11–20 | 10 | 8 | 164 | 0 | 0 | 2/8/0 (20.0% (2/10)) | 10 | 10 recorded / 0 missing | 0 | 10 recorded / 0 missing |
| 21–30 | 10 | 11 | 308 | 0 | 0 | 5/5/0 (50.0% (5/10)) | 6 | 10 recorded / 0 missing | 0 | 10 recorded / 0 missing |
| 31–40 | 10 | 7 | 284 | 0 | 0 | 2/8/0 (20.0% (2/10)) | 12 | 10 recorded / 0 missing | 1 | 10 recorded / 0 missing |
| 41–50 | 10 | 5 | 248 | 0 | 0 | 3/7/0 (30.0% (3/10)) | 9 | 7 recorded / 3 missing | 0 | 7 recorded / 3 missing |
| 51–60 | 10 | 2 | 248 | 2156 | 0 | 3/7/0 (30.0% (3/10)) | 13 | 7 recorded / 3 missing | 0 | 7 recorded / 3 missing |
| 61–70 | 10 | 8 | 196 | 0 | 0 | 4/6/0 (40.0% (4/10)) | 10 | 6 recorded / 4 missing | 0 | 6 recorded / 4 missing |
| 71–80 | 10 | 13 | 464 | 0 | 0 | 2/8/0 (20.0% (2/10)) | 8 | 8 recorded / 2 missing | 0 | 8 recorded / 2 missing |
| 81–90 | 10 | 23 | 968 | 0 | 0 | 2/8/0 (20.0% (2/10)) | 17 | 8 recorded / 2 missing | 0 | 8 recorded / 2 missing |
| 91–100 | 10 | 20 | 1188 | 0 | 0 | 2/8/0 (20.0% (2/10)) | 18 | 10 recorded / 0 missing | 1 | 10 recorded / 0 missing |

Reviewed testing/tool sessions with zero accepted matching functions and bytes may still have provided useful evidence, but receive no recovered-code/data credit in these totals.

<!-- END GENERATED LUNA ASSESSMENT -->
