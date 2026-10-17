@ Unrecovered code/data after the two selection wrappers and before coordinate lookup.
.section .text
.global rom_other_selection_suffix_start
rom_other_selection_suffix_start:
.incbin "baserom.gba", 0x995c0, 0x5c
