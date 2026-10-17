# Content selection wrappers at context offsets `+0x14` and `+0x18`

`ResolveContentSelectionAt18` and `ResolveContentSelectionAt14` recover the
32-byte wrappers at ROM offsets `0x99580–0x995A0` and `0x995A0–0x995C0`.
Each saves the selector from argument 1, initializes a local selected-entry
pointer to null, obtains the list owner from its indicated context field,
passes the `ContentSelectionEqualsObject` callback literal (`0x0809AB5D`) to
`VisitLinkedNodes` (`0x080C7CC0`), and returns the selected pointer. The
four-byte local word at stack offset `+4` is not initialized or read by the
wrapper or predicate, so its meaning remains unknown.

Both wrappers use the existing native-pointer
`ContentSelectionLookupContext`; the shared `ListCountContext` retains opaque
field names and meanings. The focused host harness compiles each wrapper,
predicate, list payload getter, and visitor as separate translation units. It
checks both selected offsets against distinct list decoys, first/middle/first
duplicate matches, empty and absent lists, null-selector/null-object pointer
equality, and complete fixture images with native pointer identities. Both
exact functions are integrated in the matching build, with their addresses
and extents guarded by the linker and the audited region ledger.

The ROM block is exactly 64 bytes. Each function retains its own 32-byte
symbol extent and address. The copied bytes following the wrappers remain a
separate short-named assembly segment before the coordinate lookup routines.
