@ Unrecovered original code/data retained around a source carve.
.section .text
.global rom_content_globals_suffix_start
rom_content_globals_suffix_start:
.incbin "baserom.gba", 0x96d94, 0x2f24
