@ Unrecovered original code/data after the selection lookup wrapper and before context payload visitors.
.section .text
.global rom_context_lookup_suffix_start
rom_context_lookup_suffix_start:
.incbin "baserom.gba", 0x99580, 0x2c0
