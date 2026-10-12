@ Bounded 7-record prefix at ROM 0x0839475C.
@ Field names reflect observed offsets only.
.section .text
.balign 4
.global gClassRecords4
.type gClassRecords4, %object
.macro class_u8 value
    .if (\value < 0) || (\value > 255)
        .error "class byte out of range"
    .endif
    .byte \value
.endm
.macro class_u16 value
    .if (\value < 0) || (\value > 65535)
        .error "class halfword out of range"
    .endif
    .hword \value
.endm
.macro class4_record id, field00, unknown01, unknown02, unknown03
    .if (. - gClassRecords4) != (\id * 4)
        .error "class4_record IDs must be consecutive from zero"
    .endif
    class_u8 \field00
    class_u8 \unknown01
    class_u8 \unknown02
    class_u8 \unknown03
.endm
gClassRecords4:
    class4_record 0, 6, 64, 0, 0
    class4_record 1, 6, 64, 0, 0
    class4_record 2, 6, 64, 0, 0
    class4_record 3, 6, 64, 0, 0
    class4_record 4, 6, 64, 0, 0
    class4_record 5, 6, 64, 0, 0
    class4_record 6, 7, 64, 0, 0
.if (. - gClassRecords4) != 28
    .error "gClassRecords4 table size changed"
.endif
.global gClassRecords4End
gClassRecords4End:
.size gClassRecords4, .-gClassRecords4
