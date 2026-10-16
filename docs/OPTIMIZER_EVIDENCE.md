# Session 62 compiler optimization evidence

This bounded experiment compares complete C translation units under agbcc `-O0`, `-O1`, `-O2`, and `-Os`. It is diagnostic compiler-shape research. No production source, production compiler flags, linker layout, or recovery ledger changed, and no nonmatching function receives recovery credit.

The run used `python3 tools/research/optimizer_matrix.py --output build/optimizer-matrix-session62.json` and executed 32 whole-TU probes. The generated JSON is an ignored build artifact and records every source hash, per-probe preprocessed-source hash, symbol-aligned metric, whole-block metric, full probe flags, and reference identity. It requires the preprocessed bytes for a TU to remain identical across all four levels, catching an included-header change during the matrix. It fingerprints the actual compiler binary and verifies that hash remains stable. The tool captures probe stdout/stderr rather than mixing it into JSON; it refuses stale output folders and requires fresh valid reports, even when `probe.py` exits with its normal mismatch status.

The compiler was pret/agbcc revision `da598c1d918402c42c0c0d7128ba14567f3175e9`, declared banner `gcc 2.9-arm-000512 for Thumb/elf`; the actual `tools/.deps/agbcc/gcc/agbcc` binary SHA-256 was `f730262fbbd459b98038d3a66e642666e08a3e97a2bf2183da7eb993e63cabf6`. Preprocessing was arm-none-eabi-cpp 15.2.0 and binutils was 2.45. Every level used `-mthumb-interwork`; preprocessing used `-P -Iinclude`, and assembly used `-mcpu=arm7tdmi`. The reference was `build/verified-278-tree-layout-ref.elf`, SHA-256 `be0a3ca7edfc3057225e11b125451376d250ea2b3c7cd52d0953a02276fbc2e4`.

The probe bounds are derived from the function manifests. Probe blocks include the verified two-byte trailing alignment for DetachTreeNode, CountTreeRootNodes, the callback TU, and AppendListNode. `python3 tools/research/optimizer_matrix.py --plan` prints the validated descriptor plan without compiling.

| Optimization | Candidate whole-block differing bytes (5 TUs) | Candidate symbol-aligned differing bytes (5 symbols) | Matching control whole-block differing bytes (3 TUs) | Exact matching control symbols (6) |
| --- | ---: | ---: | ---: | ---: |
| `-O0` | 910 | 909 | 362 | 0 |
| `-O1` | 421 | 420 | 33 | 5 |
| `-O2` | 352 | 352 | 0 | 6 |
| `-Os` | 394 | 392 | 0 | 6 |

No nonmatching candidate became an exact symbol match. `-O2` has the lowest aggregate candidate block difference and preserves all six known matching control symbols. `-Os` also preserves the controls but is 42 candidate block bytes worse overall, primarily because CountTreeRootNodes regresses from 6 differing block bytes at `-O2` to 49 at `-Os`. That root-count sensitivity is not a uniform improvement across adjacent functions: ClearContentFlag21FIfSet plateaus at 31 differences for all three optimized levels, DestroyContainer plateaus at 136, InsertTreeNode plateaus at 42 under `-O2`/`-Os`, and DetachTreeNode differs by only one byte between those levels. This evidence does not justify a production flag change or per-function optimization overrides.

Per-TU whole-block differences, including alignment bytes, were:

| Translation unit | `-O0` | `-O1` | `-O2` | `-Os` |
| --- | ---: | ---: | ---: | ---: |
| ClearContentFlag21FIfSet candidate | 44 | 31 | 31 | 31 |
| InsertTreeNode candidate | 163 | 55 | 42 | 42 |
| DetachTreeNode candidate | 320 | 146 | 137 | 136 |
| DestroyContainer candidate | 237 | 136 | 136 | 136 |
| CountTreeRootNodes candidate | 146 | 53 | 6 | 49 |
| `list_callbacks.c` controls | 155 | 13 | 0 | 0 |
| `content_index.c` controls | 98 | 20 | 0 | 0 |
| AppendListNode control | 109 | 0 | 0 | 0 |

Each block count above is separate from strict symbol-aligned results. The full JSON includes per-symbol expected/actual size, address delta, symbol-aligned differing bytes, and strict MATCH/DIFF. In particular, the 6-byte CountTreeRootNodes block result at `-O2` is still a DIFF; its function symbol itself has six differing bytes and remains unadmitted.

Source SHA-256 values observed during the run:

| Translation unit | SHA-256 |
| --- | --- |
| `src/nonmatching/content_flag_clear.c` | `ca92b8f800e3cf79ad546cab0dc7d98dd2cacb9a47a36dd47d47d96d16aa4eb8` |
| `src/nonmatching/tree_insert.c` | `1a18498a65ae7f6d929ce544d2df3ab226a4105781ea8106c738d9706e5cf92a` |
| `src/nonmatching/tree_detach.c` | `79c3174949b6ddd94bbd5574fdced852d4ca84c0c2e04d23434abfa7783ca298` |
| `src/nonmatching/container_destroy.c` | `5c9ad09a92824627467635562165daadddda06a6ace27e473ca3fd94d660c3ec` |
| `src/nonmatching/tree_root_count.c` | `9aa7f5de8da1de21fbbc0e5b33a6c884b8abbf4b0e43ed620290cadd1a167a20` |
| `src/list_callbacks.c` | `180e7ff43a54c1f281e427802dd8702033b8f44638431c6bb1dcf14978ac753f` |
| `src/content_index.c` | `4a9adfdfb10c32ad083f56c1f9cf7bc2c25a00b8d88698858971853f5c960b01` |
| `src/list_append.c` | `1f5a1a4dc8937ec104bd2217de998d68541e16c2cf5c0d250ac5746861907a07` |

Focused validation: `python3 -m unittest tests/test_optimizer_matrix.py -v` passed eight tests covering manifest-derived descriptors and bounds, source hashes, reference immutability, malformed/missing/duplicate symbol metrics, output freshness, valid mismatch exits, deterministic optimization order/results, and source immutability. No full discovery or matching-build gate was run for this research-only tooling change.
