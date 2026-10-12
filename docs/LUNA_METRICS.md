# Luna session registry and metrics

`config/luna_sessions.json` is the record of distinct Luna coding agents that
actually spawned. A follow-up correction or revision belongs to the same
session. The reporting tool validates the complete registry before producing
counts:

```sh
python3 tools/decomp/luna_sessions.py
python3 tools/decomp/luna_sessions.py --json
```

The text report prints every ten-session batch that has at least one reviewed
session. JSON retains the same `reviewed_batches_of_10` structure, including
empty batches. Planned entries are placeholders and do not count as started.
Reviewed counts require review evidence.

## Continual-learning experiment

The current target is 100 distinct Luna sessions, including the original three
pilot sessions. The parent manages three workers, reviews their evidence, and
returns corrections to the original author. The desired long-term split is
roughly 90% agent implementation with minimal parent correction; that is an
aspiration, not a measured result.

For every review correction, record both the defect and what the parent could
have supplied earlier: clearer bounds, original branch evidence, compatible
types, an independent fixture, a dependency snapshot, or a better tool. Update
`config/luna_lessons.json`, then include the relevant lesson in later task
packets. Follow-up revisions do not create additional sessions. Parent reviews
must distinguish implementation correctness, matching bytes, and honest
research that remains uncounted.

Before writing loop code, require a raw-address map of entry branches,
fallthroughs, back-edges and updates. Trace zero, one and two iterations from
the original instructions independently of the proposed source. Preserve
compiler experiments as reproducible transformations or source, not only
hashes of temporary files. Execute actual recovered dependencies where
possible; otherwise assert complete state at dependency call time.

At each integration point, freeze tested files, run the full matching gate,
and record accepted functions/code/data separately. Review ten-session batches
for recurring prompt failures and useful tools. Do not infer speedup from
concurrency or treat wall-clock lifecycle timestamps as implementation effort.

## Lifecycle recording

After an agent has successfully spawned, register that distinct agent once:

```sh
python3 tools/decomp/luna_session_record.py config/luna_sessions.json register \
  --agent /root/luna_035_example --model gpt-6-luna --task "bounded task"
```

The registration operation validates the full registry, rejects an already
used agent identity, and appends only the next contiguous ID. It never creates
an agent or counts a planned assignment. It records the current UTC time as
`registered_utc`; it does not claim that registration time is the spawn time.
If the actual spawn timestamp is known, supply `--spawned-utc` in timezone-aware
ISO format. Both the spawn and registration timestamps are then retained.
Without a supplied spawn timestamp, `started_utc` remains absent and a note
states that exact spawn timing is unavailable. Atomic replacement protects the
registry from partial writes. Atomic replacement does not prevent lost updates
when two writers read the same old registry. The parent must hold a single
writer lease for the registry across each registration or review operation;
other agents should not write it concurrently.

After independent review, provide evidence and the accepted outputs:

```sh
python3 tools/decomp/luna_session_record.py config/luna_sessions.json review \
  --id 35 --evidence "Parent reran focused tests and reviewed the source." \
  --accepted-functions 1 --accepted-code-bytes 24 --accepted-data-bytes 0 \
  --first-review-accepted true
```

The review transition accepts only a running or review-pending session. The
accepted byte total is derived from the supplied code and data byte counts;
function and byte counts are explicit review inputs. Evidence is required.
Review does not fill missing timing fields or change existing parent/agent
effort, revision, or manually recorded metadata.

## Metric boundaries

- `parent_minutes` and `agent_minutes` are measured effort. Missing values stay
  `null` and reports count them as unknown. Registration time and elapsed wall
  time never become effort values.
- `started_utc` and `finished_utc`, when both known, support elapsed wall-time
  reporting only. Optional one-sided timestamps remain valid and produce no
  elapsed measurement.
- `registered_utc` and `reviewed_utc` are optional lifecycle timestamps. If
  present, each must independently be a timezone-aware timestamp. Missing
  lifecycle timestamps remain unknown and are not filled from other fields.
- Accepted matching code and data bytes are reported separately. Any accepted
  bytes without either classification are reported as unclassified. These
  values require independent review; a task plan or agent claim alone does not
  establish accepted recovery.
- First-review acceptance, revisions, parent corrective edits, and reviewed
  accepted outputs are separate measures. Unknown first-review outcomes remain
  unknown rather than false.
- Batch comparisons summarize reviewed sessions in ID ranges of ten. Different
  task mixes and unknown effort prevent unsupported speed, cost, or effort-share
  conclusions.

The lifecycle API and tests use temporary registries. Tests must not write the
live registry; registration and review on the real file happen only after the
corresponding spawn or independent review has actually occurred.
