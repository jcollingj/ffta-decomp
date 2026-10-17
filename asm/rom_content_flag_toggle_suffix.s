@ Unrecovered original code/data retained after the content flag toggle.
.section .text
.global rom_content_flag_toggle_suffix_start
rom_content_flag_toggle_suffix_start:
.incbin "baserom.gba", 0x96e18, 0x2ea0
