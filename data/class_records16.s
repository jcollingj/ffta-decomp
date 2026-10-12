@ Bounded 86-record prefix at ROM 0x08528F34.
@ Records beyond index 85 remain unclassified/copied.
.section .text
.balign 4
.global gClassRecords16
.type gClassRecords16, %object
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
.macro class16_record id, field00, unknown01, field02, unknown04_00, unknown04_01, unknown04_02, unknown04_03, unknown04_04, unknown04_05, unknown04_06, unknown04_07, unknown04_08, unknown04_09, unknown04_10, unknown04_11
    .if (. - gClassRecords16) != (\id * 16)
        .error "class16_record IDs must be consecutive from zero"
    .endif
    class_u8 \field00
    class_u8 \unknown01
    class_u16 \field02
    class_u8 \unknown04_00
    class_u8 \unknown04_01
    class_u8 \unknown04_02
    class_u8 \unknown04_03
    class_u8 \unknown04_04
    class_u8 \unknown04_05
    class_u8 \unknown04_06
    class_u8 \unknown04_07
    class_u8 \unknown04_08
    class_u8 \unknown04_09
    class_u8 \unknown04_10
    class_u8 \unknown04_11
.endm
gClassRecords16:
    class16_record 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    class16_record 1, 6, 2, 196, 1, 16, 17, 19, 18, 0, 0, 0, 1, 1, 2, 0
    class16_record 2, 6, 1, 212, 1, 2, 4, 5, 8, 22, 6, 7, 1, 1, 1, 0
    class16_record 3, 5, 50, 232, 1, 9, 10, 11, 12, 0, 0, 0, 1, 1, 2, 0
    class16_record 4, 2, 9, 192, 1, 3, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0
    class16_record 5, 2, 15, 192, 1, 23, 0, 0, 0, 0, 0, 0, 1, 1, 2, 0
    class16_record 6, 3, 8, 224, 1, 21, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0
    class16_record 7, 5, 10, 200, 1, 15, 0, 0, 0, 0, 0, 0, 1, 1, 2, 0
    class16_record 8, 5, 6, 212, 1, 20, 0, 0, 0, 0, 0, 0, 1, 2, 1, 0
    class16_record 9, 3, 4, 224, 1, 32, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0
    class16_record 10, 4, 7, 248, 1, 13, 0, 0, 0, 0, 0, 0, 1, 1, 2, 0
    class16_record 11, 2, 14, 192, 1, 27, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0
    class16_record 12, 4, 18, 208, 1, 26, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0
    class16_record 13, 4, 19, 208, 1, 28, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0
    class16_record 14, 3, 11, 228, 1, 24, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0
    class16_record 15, 5, 5, 200, 1, 25, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0
    class16_record 16, 2, 17, 192, 1, 29, 0, 0, 0, 0, 0, 0, 1, 2, 1, 0
    class16_record 17, 5, 16, 200, 1, 30, 0, 0, 0, 0, 0, 0, 1, 2, 1, 0
    class16_record 18, 3, 12, 224, 1, 31, 0, 0, 0, 0, 0, 0, 1, 2, 1, 0
    class16_record 19, 6, 13, 196, 1, 14, 0, 0, 0, 0, 0, 0, 1, 1, 2, 0
    class16_record 20, 2, 51, 196, 1, 1, 0, 0, 0, 0, 0, 0, 1, 3, 1, 0
    class16_record 21, 1, 22, 208, 2, 1, 0, 0, 0, 0, 0, 0, 1, 4, 2, 0
    class16_record 22, 1, 21, 208, 2, 5, 0, 0, 0, 0, 0, 0, 1, 4, 2, 0
    class16_record 23, 1, 25, 208, 2, 6, 0, 0, 0, 0, 0, 0, 1, 4, 2, 0
    class16_record 24, 5, 51, 208, 2, 7, 0, 0, 0, 0, 0, 0, 1, 4, 2, 0
    class16_record 25, 1, 23, 192, 2, 2, 0, 0, 0, 0, 0, 0, 1, 4, 2, 0
    class16_record 26, 6, 27, 0, 3, 1, 0, 0, 0, 0, 0, 0, 3, 5, 3, 0
    class16_record 27, 5, 26, 32, 3, 2, 0, 0, 0, 0, 0, 0, 3, 5, 3, 0
    class16_record 28, 3, 29, 8, 3, 3, 0, 0, 0, 0, 0, 0, 3, 5, 3, 0
    class16_record 29, 1, 28, 4, 3, 4, 0, 0, 0, 0, 0, 0, 3, 5, 3, 0
    class16_record 30, 3, 33, 208, 4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0
    class16_record 31, 1, 0, 0, 5, 100, 0, 0, 0, 0, 0, 0, 0, 8, 5, 0
    class16_record 32, 1, 0, 0, 5, 50, 0, 0, 0, 0, 0, 0, 0, 8, 5, 0
    class16_record 33, 6, 30, 228, 6, 0, 0, 0, 0, 0, 0, 0, 1, 8, 4, 0
    class16_record 34, 2, 3, 200, 7, 0, 0, 0, 0, 0, 0, 0, 1, 6, 4, 0
    class16_record 35, 4, 36, 208, 8, 0, 0, 0, 0, 0, 0, 0, 1, 1, 4, 0
    class16_record 36, 5, 35, 200, 9, 0, 0, 0, 0, 0, 0, 0, 1, 1, 4, 0
    class16_record 37, 1, 52, 192, 10, 1, 0, 0, 0, 0, 0, 0, 1, 7, 6, 0
    class16_record 38, 3, 53, 228, 10, 2, 0, 0, 0, 0, 0, 0, 1, 7, 6, 0
    class16_record 39, 5, 43, 212, 10, 3, 0, 0, 0, 0, 0, 0, 1, 7, 6, 0
    class16_record 40, 5, 47, 200, 10, 4, 0, 0, 0, 0, 0, 0, 1, 7, 6, 0
    class16_record 41, 2, 44, 192, 10, 5, 0, 0, 0, 0, 0, 0, 1, 7, 6, 0
    class16_record 42, 4, 46, 196, 10, 6, 0, 0, 0, 0, 0, 0, 1, 7, 6, 0
    class16_record 43, 1, 39, 200, 10, 7, 0, 0, 0, 0, 0, 0, 1, 7, 6, 0
    class16_record 44, 2, 41, 224, 10, 8, 0, 0, 0, 0, 0, 0, 1, 7, 6, 0
    class16_record 45, 4, 54, 208, 10, 9, 0, 0, 0, 0, 0, 0, 1, 7, 6, 0
    class16_record 46, 3, 42, 224, 10, 15, 0, 0, 0, 0, 0, 0, 1, 7, 6, 0
    class16_record 47, 3, 40, 196, 10, 16, 0, 0, 0, 0, 0, 0, 1, 7, 6, 0
    class16_record 48, 4, 49, 208, 10, 17, 0, 0, 0, 0, 0, 0, 1, 7, 6, 0
    class16_record 49, 5, 48, 200, 10, 18, 0, 0, 0, 0, 0, 0, 1, 7, 6, 0
    class16_record 50, 2, 37, 224, 10, 13, 14, 19, 0, 0, 0, 0, 1, 7, 6, 0
    class16_record 51, 1, 0, 0, 2, 8, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0
    class16_record 52, 3, 0, 32, 10, 10, 0, 0, 0, 0, 0, 0, 0, 1, 6, 0
    class16_record 53, 4, 0, 20, 10, 11, 0, 0, 0, 0, 0, 0, 0, 1, 6, 0
    class16_record 54, 1, 0, 0, 10, 12, 0, 0, 0, 0, 0, 0, 1, 1, 6, 0
    class16_record 55, 2, 0, 4, 10, 0, 0, 0, 0, 0, 0, 0, 1, 1, 6, 0
    class16_record 56, 5, 0, 40, 12, 100, 0, 0, 0, 0, 0, 0, 1, 1, 5, 0
    class16_record 57, 1, 0, 0, 13, 50, 0, 0, 0, 0, 0, 0, 0, 8, 5, 0
    class16_record 58, 2, 0, 32, 13, 100, 0, 0, 0, 0, 0, 0, 0, 8, 5, 0
    class16_record 59, 5, 0, 8, 14, 50, 0, 0, 0, 0, 0, 0, 0, 8, 5, 0
    class16_record 60, 4, 0, 20, 14, 100, 0, 0, 0, 0, 0, 0, 0, 8, 5, 0
    class16_record 61, 6, 73, 96, 14, 0, 0, 0, 0, 0, 0, 0, 0, 8, 5, 0
    class16_record 62, 5, 63, 200, 15, 6, 0, 0, 0, 0, 0, 0, 0, 7, 7, 0
    class16_record 63, 4, 62, 200, 15, 8, 0, 0, 0, 0, 0, 0, 0, 7, 7, 0
    class16_record 64, 1, 69, 192, 15, 9, 0, 0, 0, 0, 0, 0, 0, 7, 7, 0
    class16_record 65, 1, 66, 192, 15, 28, 0, 0, 0, 0, 0, 0, 0, 7, 7, 0
    class16_record 66, 2, 68, 196, 15, 7, 0, 0, 0, 0, 0, 0, 0, 7, 7, 0
    class16_record 67, 3, 70, 208, 15, 27, 0, 0, 0, 0, 0, 0, 0, 7, 7, 0
    class16_record 68, 3, 65, 200, 15, 29, 0, 0, 0, 0, 0, 0, 0, 7, 7, 0
    class16_record 69, 2, 71, 224, 15, 30, 31, 0, 0, 0, 0, 0, 0, 7, 7, 0
    class16_record 70, 4, 72, 208, 15, 32, 0, 0, 0, 0, 0, 0, 0, 7, 7, 0
    class16_record 71, 3, 64, 224, 15, 22, 0, 0, 0, 0, 0, 0, 0, 7, 7, 0
    class16_record 72, 5, 67, 212, 15, 23, 0, 0, 0, 0, 0, 0, 0, 7, 7, 0
    class16_record 73, 6, 82, 196, 16, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 0
    class16_record 74, 1, 67, 192, 15, 24, 0, 0, 0, 0, 0, 0, 0, 6, 7, 0
    class16_record 75, 1, 64, 192, 15, 25, 0, 0, 0, 0, 0, 0, 0, 6, 7, 0
    class16_record 76, 6, 71, 204, 15, 21, 0, 0, 0, 0, 0, 0, 0, 6, 7, 0
    class16_record 77, 6, 78, 4, 17, 1, 23, 0, 0, 0, 0, 0, 1, 3, 8, 0
    class16_record 78, 6, 79, 16, 17, 2, 0, 0, 0, 0, 0, 0, 1, 3, 8, 0
    class16_record 79, 6, 80, 36, 17, 3, 0, 0, 0, 0, 0, 0, 1, 3, 8, 0
    class16_record 80, 6, 81, 8, 17, 4, 0, 0, 0, 0, 0, 0, 1, 3, 8, 0
    class16_record 81, 6, 77, 0, 17, 5, 0, 0, 0, 0, 0, 0, 1, 3, 8, 0
    class16_record 82, 6, 73, 204, 17, 0, 0, 0, 0, 0, 0, 0, 1, 3, 8, 0
    class16_record 83, 7, 0, 2, 19, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 0
    class16_record 84, 7, 0, 2, 20, 9, 10, 11, 12, 0, 0, 0, 1, 1, 9, 0
    class16_record 85, 7, 0, 2, 20, 30, 0, 0, 0, 0, 0, 0, 1, 2, 9, 0
.if (. - gClassRecords16) != 1376
    .error "gClassRecords16 table size changed"
.endif
.size gClassRecords16, .-gClassRecords16
