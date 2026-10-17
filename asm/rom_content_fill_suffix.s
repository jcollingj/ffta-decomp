@ Unrecovered original code/data after FillContentConsumerEntries and before the content unit selection callback.
.section .text
.global rom_content_fill_suffix_start
rom_content_fill_suffix_start:
.incbin "baserom.gba", 0x99d34, 0xe08
