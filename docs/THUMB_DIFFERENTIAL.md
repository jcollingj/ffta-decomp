# Bounded Thumb differential runner

`tools/decomp/thumb_runner.py` executes an original ROM function and a candidate
overlay in separate Unicorn ARM/Thumb instances. It is a CPU-only leaf-function
probe for catching semantic mismatches before byte matching. It does not admit
recovery credit and does not emulate the game, BIOS, callbacks, interrupts, DMA,
timing, or GBA peripherals.

## Setup

Run `python3 tools/decomp/bootstrap_thumb_runner.py`. This creates the ignored
`tools/.deps/thumb-runner` environment and installs the sole pinned dependency
from `tools/decomp/requirements-thumb-runner.txt` (Unicorn 2.1.4). The runner
uses the upstream Python `Uc` API, Thumb mode, memory-map API, instruction
count limit, and ARM register identifiers. I checked these calls against the
versioned [Unicorn ARM sample](https://github.com/unicorn-engine/unicorn/blob/2.1.4/samples/sample_arm.c),
[Python emulation API](https://github.com/unicorn-engine/unicorn/blob/2.1.4/bindings/python/unicorn/unicorn_py3/unicorn.py),
and [Python constants](https://github.com/unicorn-engine/unicorn/blob/2.1.4/bindings/python/unicorn/unicorn_const.py).

Run its ROM-independent tests with:

```sh
tools/.deps/thumb-runner/bin/python -m unittest tests.test_thumb_runner -v
```

## Scenario format

Create a local JSON file (do not commit ROM-derived/private artifacts) with a
candidate function's complete byte overlay. Addresses below are byte offsets in
the ROM unless labeled as CPU addresses.

```json
{
  "rom_path": "baserom.gba",
  "entry_offset": "0x1000",
  "overlay_offset": "0x1000",
  "original_function_size": "0x20",
  "candidate_hex": "01207047",
  "expected_sha1": "4ac05441f4de70a4ec3dd932116346c61b8783d9",
  "args": ["0x02001000"],
  "instruction_budget": 1000,
  "timeout_us": 250000,
  "ram_regions": [
    {"address": "0x02000000", "size": "0x40000", "label": "ewram"}
  ],
  "watch": [
    {"address": "0x02001000", "size": 8, "label": "owner"}
  ],
  "compare_registers": ["r0", "r1", "r2", "r3"]
}
```

Then run:

```sh
tools/.deps/thumb-runner/bin/python tools/decomp/thumb_runner.py scenario.json
```

The USA SHA-1 is checked before execution. ROM is mapped at `0x08000000` with
read/execute permissions, and candidate bytes are overlaid only in the
candidate instance. CLI scenarios must use that fixed ROM base. The default
stack is the explicit 32 KiB IWRAM range at `0x03000000`; r0-r3 receive the
listed arguments, and remaining general registers start with distinct
sentinels so accidental clobbers are visible. LR points to a return sentinel.
EWRAM is not mapped unless
the scenario declares it. RAM mappings must stay within supported EWRAM or
IWRAM, and extra regions must be page aligned. BIOS, MMIO, video memory, and
other unmapped accesses fail. Unicorn has no ARM7TDMI model, so it runs its
TI925T ARMv4T model as the closest available instruction-set model. It rejects 32-bit Thumb
encodings, ARM-state transitions, BL/BLX calls, and execution outside the
declared original or candidate function extent. `original_function_size`
defaults to the candidate overlay length and should be set from independent
function-boundary evidence when they differ.

The instruction count is a hard per-run cap. `MATCH` means only that the
selected register values and watched memory bytes agree. At least one
observable register or memory range must be selected. There is no register
comparison default: r0-r3 are volatile under the calling convention, so select
them only when the function's return or output contract makes them observable.
Use callee-saved registers or stack pointer only when their preservation is
part of the assertion. `DIFF` includes the specific register or memory range
plus both observed values. The CLI exits 1 for a behavioral `DIFF`, 2 for
invalid setup or execution, and 0 for `MATCH`. Each run has both an instruction
count cap and a wall-clock timeout. The report includes each run's instruction
count and measured emulation time. Setup time (ROM hash, mapping, and state
initialization) is excluded from those per-run times. Unsupported instructions,
invalid return, unmapped access, or attempt to leave the leaf extent is an
error, not a pass or a `DIFF`.

On the local Apple Silicon / Python 3.14.6 environment, a synthetic 16 MiB ROM
with a two-instruction leaf returned `MATCH` in nine measured differential
pairs. End-to-end median was 19.064 ms per pair (range 18.337–32.105 ms); the
median emulation time was 86.9 microseconds per variant (range 58.5–1384.0
microseconds). End-to-end time includes reading and hashing the 16 MiB file and
mapping both CPU instances. This is a local latency measurement, not a speedup
claim or a measurement on a real allocator function.

## Scope and limits

This is not a general GBA emulator. It cannot establish correctness for code
that depends on a callback, another function, BIOS service, peripheral, DMA,
interrupt, or device state. Split such code at a genuinely leaf boundary or
use a separate, explicitly justified fixture. The stack and RAM bytes are
synthetic inputs and must be initialized from independently traced register
and memory provenance. Watches should cover every caller-owned byte that the
function may change, not just the field expected to move. Verify that pointer
bases, scales, and loop back-edges come from original instructions before
constructing the fixture. Candidate-vs-original agreement can still preserve a
shared wrong interpretation; it supplements independent disassembly and
fixtures rather than replacing them.

An optional local-ROM test executes the original `AllocatorRecenter` at
`0x08006D38` with the independently documented `0x6D38–0x6D9E` extent and
counts 0–4. It expects counts 0–3 to retain the head word offset 8, and count 4
to follow one link to offset 12. The candidate overlay is identical to the
original bytes in this fixture, so it establishes the original's first-hop
behavior only; the synthetic differential test separately proves register and
memory `DIFF` reporting. The candidate remains nonmatching and uncounted.

The synthetic tests also exercise Thumb-2, ARM-state, BL/BLX, and SWI rejection,
unsupported instruction rejection, out-of-extent execution, unmapped writes, MMIO-map
rejection, return handling, wall-clock timeout and instruction-budget
exhaustion, ROM hash checks, and input-file immutability. The real-ROM
test skips when the local ROM is absent or when tests run with a Python that
does not have Unicorn installed.
