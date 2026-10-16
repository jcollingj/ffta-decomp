@ Unrecovered original code/data retained around a source carve.
.section .text
.global rom_random_prefix_start
rom_random_prefix_start:
.incbin "baserom.gba", 0xa040, 0x21a00
