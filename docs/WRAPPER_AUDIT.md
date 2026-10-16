# Copied ROM wrapper audit

`tools/decomp/wrapper_audit.py` checks the reserved `asm/rom*.s` namespace for
copied ROM wrappers that remain after source integration. It is read-only: it
does not remove files, edit the ledger or Makefile, build the ROM, or decide
whether an unreferenced file is safe to delete.

The audit first validates the complete `config/decomp_regions.json` region
partition using the same coverage validator as `make progress`, then parses the
explicit Makefile `OBJS` assignment and resolves every active object to one
source. These are independent authorities. If a copied ledger source is absent
from the active object list, an active copied wrapper has no copied ledger
entry, or an active object has no unique source, the report identifies the
mismatch and does not infer that the remaining files are orphans.

The audited namespace is restricted to files directly under `asm/` named
`rom*.s`. A content-bearing file counts as a copied wrapper only when its
meaningful lines are recognized `.incbin "baserom.gba", OFFSET, LENGTH`
directives and simple wrapper scaffolding (`.section .text`, `.global`, labels,
or alignment). Authored assembly with instructions or other content stays out
of scope. A whitespace-empty file in this reserved namespace is reported as an
empty candidate when neither the copied ledger nor active objects reference
it. Nonempty candidates are reported separately. Any deletion still requires
review of repository consumers and ownership.

## First cleanup report

On 2026-10-16 the audit found two nonempty wrappers, `asm/rom_ability_setter.s`
and `asm/rom_unit_tail.s`. Both contained only `.section .text` and a
`baserom.gba` `.incbin`; neither path appeared in the copied ledger or active
`OBJS`. A repository-wide reference search found no other consumers. The two
files were removed, leaving `baserom.gba` unchanged. After cleanup, the focused
audit reported no orphan candidates or ledger/build mismatches.

Run the focused tests and audit with:

```sh
python3 -m unittest tests.test_wrapper_audit -v
python3 tools/decomp/wrapper_audit.py
python3 tools/decomp/wrapper_audit.py --json
```

The CLI exits 0 when no orphan or consistency finding exists, 1 when the audit
finds wrappers or ledger/build mismatches, and 2 when it cannot validate its
authoritative inputs. JSON mode always emits a parseable report; an invalid
ledger produces an error report with no orphan verdict. The tool deliberately
does not embed expected live counts, since the active ledger and object list
change as recovery progresses.
