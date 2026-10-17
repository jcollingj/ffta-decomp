@ Unrecovered original code/data before FillContentConsumerEntries.
.section .text
.global rom_content_fill_prefix_start
rom_content_fill_prefix_start:
.incbin "baserom.gba", 0x99cdc, 0x2c
