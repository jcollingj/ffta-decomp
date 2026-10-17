@ Unrecovered original code/data after the selection predicate and before the payload adapter.
.section .text
.global rom_selection_lookup_suffix_start
rom_selection_lookup_suffix_start:
.incbin "baserom.gba", 0x9ab7c, 0x6c
