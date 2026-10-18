@ Unrecovered bytes after the content coordinate forwarder.
.section .text
.global rom_context_visit_tail_start
rom_context_visit_tail_start:
.incbin "baserom.gba", 0x99C30, 0x88
