@ Unrecovered bytes after SetContentEntryPosition.
.section .text
.global rom_content_position_tail_start
rom_content_position_tail_start:
.incbin "baserom.gba", 0x99EA0, 0x178
