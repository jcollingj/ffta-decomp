@ Unrecovered original code/data retained around a source carve.
.section .text
.global rom_content_globals_prefix_start
rom_content_globals_prefix_start:
.incbin "baserom.gba", 0x2baf8, 0x6b284
