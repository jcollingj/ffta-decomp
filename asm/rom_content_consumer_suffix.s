@ Unrecovered original code/data retained around a source carve.
.section .text
.global rom_content_consumer_suffix_start
rom_content_consumer_suffix_start:
.incbin "baserom.gba", 0x2baf8, 0x6e1c0
