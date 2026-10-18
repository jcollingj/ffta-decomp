@ Unrecovered original code/data after the context content collectors.
.section .text
.global rom_content_collect_suffix_start
rom_content_collect_suffix_start:
.incbin "baserom.gba", 0x99e68, 0xcd4
