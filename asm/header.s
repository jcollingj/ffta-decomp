@ GBA cartridge header (0x000000-0x0000C0)
@ First region carved out of baserom.gba into editable source.

    .section .text
    .global _start
_start:
    .incbin "baserom.gba", 0x000, 0x0A0 @ entry branch + Nintendo logo

    .ascii "FFTA_USVER.\0"              @ 0x0A0: internal title (12 bytes)
    .ascii "AFXE"                       @ 0x0AC: game code (USA)
    .ascii "01"                         @ 0x0B0: maker code (Nintendo)
    .byte  0x96                         @ 0x0B2: fixed value
    .byte  0x00                         @ 0x0B3: main unit code
    .byte  0x00                         @ 0x0B4: device type
    .space 7                            @ 0x0B5: reserved
    .byte  0x00                        @ 0x0BC: software version
    .byte  0x00                         @ 0x0BD: header checksum (patched by tools/gbafix.py)
    .space 2                            @ 0x0BE: reserved
