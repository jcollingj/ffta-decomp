# Decompiled C manifest invariants

The region ledger and function inventory make separate claims. A `decompiled_c`
region attributes ROM bytes to a C source file, while a `matching_c` function
entry identifies a recovered symbol whose bytes and layout are checked by the
comparison tools. Progress validation checks that these claims agree in both
directions:

- Every `decompiled_c` region names a source and contains at least one
  `matching_c` function wholly inside its bounds with the same source path.
- Every `matching_c` function names a source and is wholly inside a
  `decompiled_c` region with that same source path.
- A function that crosses a region boundary, has no source, or names another
  source is rejected. A C region with no matching function is rejected.

Region bounds can include literal pools, switch tables, and alignment bytes
needed by the matching build. Those bytes remain part of ROM provenance
coverage; the validator does not require function symbols to cover every byte
in a C region or assign padding to a function. Multiple functions may belong to
one source region, with gaps between their symbol bounds.

This validation checks manifest consistency. It does not discover functions,
prove that a C source implements a symbol, or infer a complete function
inventory. If `code_inventory_complete` is false, the total function count and
code completion percentage remain unknown. The normal audit separately checks
that declared source files exist, that source-backed regions are included in
the matching build, and that copied regions agree with `.incbin` directives.
