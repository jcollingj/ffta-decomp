@ Original 376 item records, 32 bytes each. Field names follow include/item.h.
@ Unknown fields are preserved by offset; numeric property names are reader selectors.
.section .text
.global gItems
.type gItems, %object
.macro item_u8 value
    .if (\value < 0) || (\value > 255)
        .error "item_u8 value out of range"
    .endif
    .byte \value
.endm
.macro item_s8 value
    .if (\value < -128) || (\value > 127)
        .error "item_s8 value out of range"
    .endif
    .byte \value
.endm
.macro item_u16 value
    .if (\value < 0) || (\value > 65535)
        .error "item_u16 value out of range"
    .endif
    .hword \value
.endm
.macro item_header id, name, property01, property02, unknown06, unknown07
    .if (. - gItems) != (\id * 32)
        .error "item IDs must be consecutive from zero"
    .endif
    item_u16 \name
    item_u16 \property01
    item_u16 \property02
    item_u8 \unknown06
    item_u8 \unknown07
.endm
.macro item_properties property03, property04, property05, property06, property07, property08, property09
    .if ((. - gItems) % 32) != 8
        .error "item field groups out of order"
    .endif
    item_u8 \property03
    item_u8 \property04
    item_u8 \property05
    item_u8 \property06
    item_u8 \property07
    item_u8 \property08
    item_u16 \property09
.endm
.macro item_modifiers modifier0, modifier1, modifier2, modifier3, modifier4, modifier5, modifier6, modifier7
    .if ((. - gItems) % 32) != 16
        .error "item field groups out of order"
    .endif
    item_s8 \modifier0
    item_s8 \modifier1
    item_s8 \modifier2
    item_s8 \modifier3
    item_s8 \modifier4
    item_s8 \modifier5
    item_s8 \modifier6
    item_s8 \modifier7
.endm
.macro item_tail unknown18, unknown19, unknown1a, unknown1b, unknown1c, ability_set, unknown1e, unknown1f
    .if ((. - gItems) % 32) != 24
        .error "item field groups out of order"
    .endif
    .if (\ability_set < 0) || (\ability_set >= 225)
        .error "item ability set must reference 0..224"
    .endif
    item_u8 \unknown18
    item_u8 \unknown19
    item_u8 \unknown1a
    item_u8 \unknown1b
    item_u8 \unknown1c
    item_u8 \ability_set
    item_u8 \unknown1e
    item_u8 \unknown1f
.endm
gItems:
    item_header 0, 0, 0, 0, 0, 0
    item_properties 0, 0, 1, 1, 0, 0, 0
    item_modifiers 17, 0, 0, 0, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 1, 124, 707, 300, 150, 0
    item_properties 1, 0, 1, 1, 123, 0, 124
    item_modifiers 25, 0, 0, 0, 0, 0, 0, 0
    item_tail 1, 0, 0, 0, 0, 1, 0, 0

    item_header 2, 125, 708, 900, 194, 1
    item_properties 1, 0, 1, 1, 123, 0, 124
    item_modifiers 30, 0, 0, 0, 2, 2, 0, 0
    item_tail 1, 0, 0, 0, 0, 2, 0, 0

    item_header 3, 126, 709, 1600, 32, 3
    item_properties 1, 0, 1, 1, 107, 0, 124
    item_modifiers 35, 5, 0, 0, 0, 0, 0, 0
    item_tail 1, 0, 0, 0, 0, 3, 0, 0

    item_header 4, 127, 710, 3000, 220, 5
    item_properties 1, 0, 1, 1, 75, 1, 124
    item_modifiers 39, 0, 0, 0, 0, 0, 0, 0
    item_tail 1, 0, 0, 0, 0, 4, 0, 0

    item_header 5, 128, 711, 0, 244, 1
    item_properties 1, 2, 1, 1, 3, 0, 126
    item_modifiers 36, 0, 0, 0, 1, 0, 0, 0
    item_tail 1, 0, 0, 0, 0, 5, 0, 0

    item_header 6, 129, 712, 0, 232, 3
    item_properties 1, 0, 1, 1, 3, 1, 126
    item_modifiers 18, 0, 0, 0, 0, 0, 0, 0
    item_tail 1, 0, 61, 0, 0, 6, 0, 0

    item_header 7, 130, 713, 0, 208, 7
    item_properties 1, 0, 1, 1, 3, 0, 124
    item_modifiers 40, 0, 5, 5, 0, 0, 0, 0
    item_tail 1, 0, 0, 0, 0, 7, 0, 0

    item_header 8, 131, 714, 0, 160, 15
    item_properties 1, 7, 1, 1, 3, 2, 124
    item_modifiers 38, 0, 2, 0, 0, 2, 0, 0
    item_tail 1, 0, 31, 0, 0, 8, 0, 0

    item_header 9, 132, 715, 0, 208, 7
    item_properties 1, 0, 1, 1, 131, 0, 124
    item_modifiers 33, 0, 0, 0, 0, 0, 0, 1
    item_tail 30, 0, 0, 0, 0, 9, 0, 0

    item_header 10, 133, 716, 0, 244, 1
    item_properties 1, 0, 1, 1, 3, 2, 124
    item_modifiers 33, 10, 0, 10, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 11, 134, 717, 0, 244, 1
    item_properties 1, 0, 1, 1, 3, 2, 126
    item_modifiers 29, 5, 0, 5, 0, 10, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 12, 135, 718, 0, 244, 1
    item_properties 1, 0, 1, 1, 3, 2, 124
    item_modifiers 65, 0, 0, 0, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 13, 136, 719, 0, 244, 1
    item_properties 1, 5, 1, 1, 3, 0, 126
    item_modifiers 41, 0, 5, 0, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 14, 137, 720, 1500, 238, 2
    item_properties 2, 0, 1, 1, 123, 0, 124
    item_modifiers 28, 0, 0, 0, 0, 0, 0, 0
    item_tail 2, 0, 0, 0, 0, 10, 0, 0

    item_header 15, 138, 721, 2500, 226, 4
    item_properties 2, 0, 1, 1, 107, 1, 125
    item_modifiers 32, 0, 0, 0, 0, 2, 0, 0
    item_tail 2, 0, 0, 0, 0, 11, 0, 0

    item_header 16, 139, 722, 3000, 220, 5
    item_properties 2, 0, 1, 1, 107, 2, 126
    item_modifiers 37, 0, 0, 0, 2, 0, 0, 0
    item_tail 2, 0, 0, 0, 0, 12, 0, 0

    item_header 17, 140, 723, 4000, 208, 7
    item_properties 2, 6, 1, 1, 107, 2, 126
    item_modifiers 36, 0, 0, 0, 0, 0, 0, 0
    item_tail 2, 0, 0, 0, 0, 13, 0, 0

    item_header 18, 141, 724, 5000, 196, 9
    item_properties 2, 1, 1, 1, 75, 1, 126
    item_modifiers 38, 0, 0, 0, 0, 0, 0, 0
    item_tail 2, 0, 0, 0, 0, 14, 0, 0

    item_header 19, 142, 725, 7000, 172, 13
    item_properties 2, 2, 1, 1, 75, 2, 124
    item_modifiers 40, 0, 0, 0, 0, 0, 0, 0
    item_tail 2, 0, 7, 0, 0, 15, 0, 0

    item_header 20, 143, 726, 8000, 160, 15
    item_properties 2, 5, 1, 1, 75, 0, 126
    item_modifiers 42, 0, 0, 0, 0, 0, 0, 0
    item_tail 2, 0, 0, 0, 0, 16, 0, 0

    item_header 21, 144, 727, 10000, 136, 19
    item_properties 2, 0, 1, 1, 75, 1, 124
    item_modifiers 40, 3, 0, 3, 0, 0, 0, 0
    item_tail 2, 0, 0, 0, 0, 17, 0, 0

    item_header 22, 145, 728, 15000, 76, 29
    item_properties 2, 0, 1, 1, 75, 1, 126
    item_modifiers 42, 0, 2, 0, 0, 0, 0, 0
    item_tail 2, 0, 0, 0, 0, 18, 0, 0

    item_header 23, 146, 729, 0, 208, 7
    item_properties 2, 0, 1, 1, 3, 1, 127
    item_modifiers 46, 0, 0, 0, 0, 0, 0, 0
    item_tail 2, 0, 0, 0, 0, 19, 0, 0

    item_header 24, 147, 730, 22000, 248, 42
    item_properties 2, 0, 1, 1, 75, 2, 127
    item_modifiers 33, 0, 10, 0, 0, 2, 0, 0
    item_tail 2, 0, 0, 0, 0, 20, 0, 0

    item_header 25, 148, 731, 0, 184, 11
    item_properties 2, 1, 1, 1, 3, 1, 126
    item_modifiers 45, 0, 2, 0, 2, 0, 0, 0
    item_tail 2, 0, 1, 18, 0, 21, 0, 0

    item_header 26, 149, 732, 0, 244, 1
    item_properties 2, 0, 1, 1, 3, 2, 125
    item_modifiers 17, 0, 15, 10, 0, 0, 0, 0
    item_tail 2, 0, 0, 0, 0, 22, 0, 0

    item_header 27, 150, 733, 0, 208, 7
    item_properties 2, 0, 1, 1, 131, 0, 124
    item_modifiers 32, 0, 0, 0, 0, 0, 0, 1
    item_tail 30, 0, 0, 0, 0, 23, 0, 0

    item_header 28, 151, 734, 0, 244, 1
    item_properties 2, 8, 1, 1, 3, 1, 125
    item_modifiers 84, 5, 0, 0, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 29, 152, 735, 0, 244, 1
    item_properties 2, 0, 1, 1, 3, 2, 127
    item_modifiers 65, 15, 0, 0, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 30, 153, 736, 0, 244, 1
    item_properties 2, 0, 1, 1, 3, 1, 126
    item_modifiers 62, 10, 0, 0, 2, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 31, 154, 737, 0, 244, 1
    item_properties 2, 0, 1, 1, 3, 0, 126
    item_modifiers 51, 0, 0, 10, 0, 2, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 32, 155, 738, 1000, 244, 1
    item_properties 3, 0, 1, 1, 107, 0, 124
    item_modifiers 25, 0, 0, 0, 2, 0, 0, 0
    item_tail 3, 0, 0, 0, 0, 24, 0, 0

    item_header 33, 156, 739, 2200, 76, 4
    item_properties 3, 0, 1, 1, 107, 2, 126
    item_modifiers 31, 0, 0, 0, 0, 2, 0, 0
    item_tail 3, 0, 0, 0, 0, 0, 0, 0

    item_header 34, 157, 740, 3700, 58, 7
    item_properties 3, 4, 1, 1, 75, 0, 126
    item_modifiers 36, 0, 0, 0, 0, 6, 0, 0
    item_tail 3, 0, 0, 0, 0, 0, 0, 0

    item_header 35, 158, 741, 0, 232, 3
    item_properties 3, 0, 1, 1, 3, 1, 126
    item_modifiers 42, 0, 5, 0, 0, 0, 0, 0
    item_tail 3, 0, 0, 0, 0, 0, 0, 0

    item_header 36, 159, 742, 0, 244, 1
    item_properties 3, 0, 1, 1, 3, 0, 127
    item_modifiers 47, 0, 10, 0, 0, 3, 0, 0
    item_tail 3, 0, 0, 0, 0, 0, 0, 0

    item_header 37, 160, 743, 0, 208, 7
    item_properties 3, 0, 1, 1, 131, 0, 124
    item_modifiers 32, 0, 0, 0, 0, 0, 0, 1
    item_tail 30, 0, 0, 0, 0, 25, 0, 0

    item_header 38, 161, 744, 0, 184, 11
    item_properties 3, 0, 1, 1, 3, 0, 124
    item_modifiers 55, 10, 0, 10, 2, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 39, 162, 745, 0, 244, 1
    item_properties 3, 1, 1, 1, 3, 2, 126
    item_modifiers 39, 0, 0, 10, 0, 5, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 40, 163, 746, 5000, 196, 9
    item_properties 4, 0, 1, 1, 75, 0, 124
    item_modifiers 37, 0, 0, 0, 0, 0, 0, 0
    item_tail 4, 0, 0, 0, 0, 26, 0, 0

    item_header 41, 164, 747, 3000, 220, 5
    item_properties 4, 8, 1, 1, 107, 1, 127
    item_modifiers 32, 0, 0, 0, 0, 0, 0, 0
    item_tail 4, 0, 0, 0, 0, 27, 0, 0

    item_header 42, 165, 748, 4000, 208, 7
    item_properties 4, 0, 1, 1, 107, 0, 126
    item_modifiers 34, 2, 0, 1, 0, 0, 0, 0
    item_tail 4, 0, 0, 0, 0, 28, 0, 0

    item_header 43, 166, 749, 7000, 172, 13
    item_properties 4, 0, 1, 1, 107, 2, 126
    item_modifiers 36, 0, 5, 0, 0, 0, 0, 0
    item_tail 4, 0, 0, 0, 0, 29, 0, 0

    item_header 44, 167, 750, 12000, 112, 23
    item_properties 4, 0, 1, 1, 75, 1, 124
    item_modifiers 46, 0, 0, 0, 0, 0, 0, 0
    item_tail 4, 0, 0, 0, 0, 30, 0, 0

    item_header 45, 168, 751, 0, 208, 7
    item_properties 4, 7, 1, 1, 3, 0, 124
    item_modifiers 45, 3, 0, 3, 0, 5, 0, 0
    item_tail 4, 0, 35, 0, 0, 31, 0, 0

    item_header 46, 169, 752, 0, 160, 15
    item_properties 4, 0, 1, 1, 3, 1, 127
    item_modifiers 48, 0, 0, 0, 0, 0, 0, 0
    item_tail 4, 0, 0, 0, 0, 32, 0, 0

    item_header 47, 170, 753, 0, 88, 27
    item_properties 4, 7, 1, 1, 3, 2, 126
    item_modifiers 47, 0, 2, 0, 1, 5, 0, 0
    item_tail 4, 0, 31, 35, 0, 33, 0, 0

    item_header 48, 171, 754, 0, 208, 7
    item_properties 4, 0, 1, 1, 131, 0, 124
    item_modifiers 32, 0, 0, 0, 0, 0, 0, 1
    item_tail 30, 0, 0, 0, 0, 34, 0, 0

    item_header 49, 172, 755, 0, 244, 1
    item_properties 4, 0, 1, 1, 3, 2, 126
    item_modifiers 87, 0, 3, 0, 4, 5, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 50, 173, 756, 0, 244, 1
    item_properties 4, 0, 1, 1, 3, 2, 126
    item_modifiers 75, 0, 0, 0, 6, 0, 1, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 51, 174, 757, 0, 244, 1
    item_properties 4, 0, 1, 1, 3, 0, 124
    item_modifiers 32, 2, 5, 2, 2, 2, 1, 1
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 52, 175, 758, 900, 194, 1
    item_properties 5, 0, 1, 2, 124, 0, 128
    item_modifiers 30, 0, 0, 0, 0, 0, 0, 0
    item_tail 5, 0, 0, 0, 0, 35, 0, 0

    item_header 53, 176, 759, 2000, 232, 3
    item_properties 5, 0, 1, 2, 108, 2, 129
    item_modifiers 32, 0, 0, 0, 0, 0, 0, 0
    item_tail 5, 0, 52, 0, 0, 36, 0, 0

    item_header 54, 177, 760, 0, 88, 2
    item_properties 5, 0, 1, 2, 4, 0, 130
    item_modifiers 32, 0, 0, 0, 0, 0, 0, 0
    item_tail 5, 0, 43, 0, 0, 37, 0, 0

    item_header 55, 178, 761, 0, 176, 4
    item_properties 5, 0, 1, 2, 4, 1, 131
    item_modifiers 42, 0, 0, 0, 0, 0, 0, 0
    item_tail 5, 0, 50, 0, 0, 0, 0, 0

    item_header 56, 179, 762, 0, 208, 7
    item_properties 5, 0, 1, 2, 4, 1, 129
    item_modifiers 37, 0, 0, 0, 0, 0, 0, 0
    item_tail 5, 0, 49, 0, 0, 38, 0, 0

    item_header 57, 180, 763, 0, 244, 1
    item_properties 5, 0, 1, 2, 4, 2, 129
    item_modifiers 49, 0, 0, 0, 0, 0, 0, 0
    item_tail 0, 0, 55, 0, 0, 0, 0, 0

    item_header 58, 181, 764, 0, 244, 1
    item_properties 5, 0, 1, 2, 4, 1, 131
    item_modifiers 59, 0, 0, 0, 0, 0, 0, 0
    item_tail 0, 0, 53, 0, 0, 0, 0, 0

    item_header 59, 182, 765, 0, 244, 1
    item_properties 5, 0, 1, 2, 4, 0, 128
    item_modifiers 48, 0, 0, 0, 0, 0, 0, 0
    item_tail 0, 0, 44, 0, 0, 0, 0, 0

    item_header 60, 183, 766, 0, 244, 1
    item_properties 5, 5, 1, 2, 4, 0, 130
    item_modifiers 45, 0, 0, 0, 0, 0, 0, 0
    item_tail 0, 0, 54, 2, 0, 0, 0, 0

    item_header 61, 184, 767, 0, 244, 1
    item_properties 5, 0, 1, 2, 4, 0, 128
    item_modifiers 51, 0, 0, 0, 0, 0, 0, 0
    item_tail 0, 0, 51, 0, 0, 0, 0, 0

    item_header 62, 185, 768, 2500, 226, 4
    item_properties 6, 3, 1, 2, 108, 2, 130
    item_modifiers 32, 5, 0, 0, 0, 0, 0, 0
    item_tail 6, 0, 0, 0, 0, 39, 0, 0

    item_header 63, 186, 769, 1200, 88, 2
    item_properties 6, 0, 1, 2, 124, 2, 128
    item_modifiers 27, 5, 0, 0, 0, 0, 0, 0
    item_tail 6, 0, 0, 0, 0, 40, 0, 0

    item_header 64, 187, 770, 4000, 208, 7
    item_properties 6, 0, 1, 2, 76, 1, 130
    item_modifiers 37, 5, 0, 0, 0, 0, 0, 0
    item_tail 6, 0, 0, 0, 0, 41, 0, 0

    item_header 65, 188, 771, 5000, 196, 9
    item_properties 6, 0, 1, 2, 76, 0, 131
    item_modifiers 33, 5, 0, 0, 0, 0, 0, 0
    item_tail 6, 0, 0, 0, 0, 42, 0, 0

    item_header 66, 189, 772, 0, 32, 3
    item_properties 6, 0, 1, 2, 4, 1, 131
    item_modifiers 47, 10, 0, 0, 0, 0, 0, 0
    item_tail 6, 0, 0, 0, 0, 43, 0, 0

    item_header 67, 190, 773, 0, 184, 11
    item_properties 6, 0, 1, 2, 4, 0, 128
    item_modifiers 49, 5, 0, 0, 0, 0, 0, 0
    item_tail 6, 0, 0, 0, 0, 44, 0, 0

    item_header 68, 191, 774, 0, 244, 1
    item_properties 6, 6, 1, 2, 4, 1, 129
    item_modifiers 45, 5, 0, 5, 0, 0, 0, 0
    item_tail 6, 0, 30, 0, 0, 45, 0, 0

    item_header 69, 192, 775, 0, 244, 1
    item_properties 6, 0, 1, 2, 4, 0, 128
    item_modifiers 47, 5, 0, 0, 2, 2, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 70, 193, 776, 0, 244, 1
    item_properties 6, 0, 1, 2, 4, 0, 128
    item_modifiers 57, 5, 0, 0, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 71, 194, 777, 0, 244, 1
    item_properties 6, 0, 1, 2, 4, 0, 128
    item_modifiers 50, 5, 0, 0, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 72, 195, 778, 0, 244, 1
    item_properties 6, 0, 1, 2, 4, 0, 128
    item_modifiers 76, 5, 5, 0, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 73, 196, 779, 25000, 244, 1
    item_properties 6, 0, 1, 2, 4, 1, 131
    item_modifiers 77, 5, 0, 5, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 74, 197, 780, 200, 100, 0
    item_properties 7, 0, 1, 1, 123, 0, 132
    item_modifiers 22, 0, 0, 0, 0, 1, 0, 0
    item_tail 7, 0, 0, 0, 0, 46, 0, 0

    item_header 75, 198, 781, 3000, 220, 5
    item_properties 7, 0, 1, 1, 75, 1, 132
    item_modifiers 30, 0, 0, 5, 0, 1, 0, 0
    item_tail 7, 0, 0, 0, 0, 47, 0, 0

    item_header 76, 199, 782, 5000, 196, 9
    item_properties 7, 0, 1, 1, 75, 0, 132
    item_modifiers 37, 0, 0, 0, 2, 1, 0, 0
    item_tail 7, 0, 0, 0, 0, 48, 0, 0

    item_header 77, 200, 783, 7000, 172, 13
    item_properties 7, 0, 1, 1, 75, 0, 132
    item_modifiers 35, 0, 0, 0, 0, 2, 0, 0
    item_tail 7, 0, 0, 0, 0, 49, 0, 0

    item_header 78, 201, 784, 900, 194, 1
    item_properties 7, 0, 1, 1, 123, 1, 133
    item_modifiers 29, 0, 0, 0, 0, 1, 0, 0
    item_tail 7, 0, 0, 0, 0, 50, 0, 0

    item_header 79, 202, 785, 10000, 136, 19
    item_properties 7, 0, 1, 1, 75, 0, 132
    item_modifiers 33, 0, 0, 0, 0, 1, 0, 0
    item_tail 7, 0, 45, 46, 0, 51, 0, 0

    item_header 80, 203, 786, 1700, 82, 3
    item_properties 7, 0, 1, 1, 123, 2, 132
    item_modifiers 31, 0, 2, 0, 0, 1, 0, 0
    item_tail 7, 0, 0, 0, 0, 52, 0, 0

    item_header 81, 204, 787, 0, 232, 3
    item_properties 7, 0, 1, 1, 3, 0, 132
    item_modifiers 38, 0, 0, 0, 1, 1, 0, 0
    item_tail 7, 0, 0, 0, 0, 0, 0, 0

    item_header 82, 205, 788, 0, 208, 7
    item_properties 7, 0, 1, 1, 3, 0, 132
    item_modifiers 39, 0, 0, 0, 0, 2, 0, 0
    item_tail 7, 0, 0, 0, 0, 53, 0, 0

    item_header 83, 206, 789, 0, 160, 15
    item_properties 7, 0, 1, 1, 3, 2, 133
    item_modifiers 60, 0, 2, 0, 0, 1, 0, 0
    item_tail 7, 0, 0, 0, 0, 54, 0, 0

    item_header 84, 207, 790, 0, 244, 1
    item_properties 7, 0, 1, 1, 3, 1, 133
    item_modifiers 57, 0, 0, 0, 5, 2, 0, 0
    item_tail 7, 0, 0, 0, 0, 55, 0, 0

    item_header 85, 208, 791, 0, 208, 7
    item_properties 7, 0, 1, 1, 131, 0, 132
    item_modifiers 32, 0, 0, 0, 0, 1, 0, 1
    item_tail 30, 0, 0, 0, 0, 56, 0, 0

    item_header 86, 209, 792, 0, 244, 1
    item_properties 7, 0, 1, 1, 3, 0, 132
    item_modifiers 37, 0, 0, 0, 10, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 87, 210, 793, 27000, 244, 1
    item_properties 7, 0, 1, 1, 3, 1, 133
    item_modifiers 35, 0, 0, 0, 15, 0, 0, 0
    item_tail 0, 0, 43, 0, 0, 0, 0, 0

    item_header 88, 211, 794, 400, 200, 0
    item_properties 8, 0, 1, 1, 123, 2, 163
    item_modifiers 25, 0, 0, 0, 2, 0, 0, 0
    item_tail 8, 0, 0, 0, 0, 57, 0, 0

    item_header 89, 212, 795, 3000, 220, 5
    item_properties 8, 0, 1, 1, 107, 1, 163
    item_modifiers 32, 0, 0, 0, 2, 0, 0, 0
    item_tail 8, 0, 0, 0, 0, 58, 0, 0

    item_header 90, 213, 796, 800, 144, 1
    item_properties 8, 0, 1, 1, 123, 0, 161
    item_modifiers 27, 0, 0, 0, 2, 0, 0, 0
    item_tail 8, 0, 0, 0, 0, 59, 0, 0

    item_header 91, 214, 797, 1500, 238, 2
    item_properties 8, 1, 1, 1, 123, 1, 162
    item_modifiers 27, 0, 2, 0, 2, 0, 0, 0
    item_tail 8, 0, 0, 0, 0, 60, 0, 0

    item_header 92, 215, 798, 5000, 196, 9
    item_properties 8, 0, 1, 1, 107, 2, 162
    item_modifiers 35, 5, 0, 0, 2, 0, 0, 0
    item_tail 8, 0, 0, 0, 0, 61, 0, 0

    item_header 93, 216, 799, 0, 244, 1
    item_properties 8, 0, 1, 1, 3, 0, 161
    item_modifiers 35, 0, 0, 0, 2, 0, 0, 0
    item_tail 8, 0, 0, 0, 0, 62, 0, 0

    item_header 94, 217, 800, 0, 232, 3
    item_properties 8, 2, 1, 1, 3, 0, 161
    item_modifiers 34, 0, 2, 0, 2, 2, 0, 0
    item_tail 8, 0, 10, 7, 0, 63, 0, 0

    item_header 95, 218, 801, 0, 208, 7
    item_properties 8, 0, 1, 1, 3, 2, 162
    item_modifiers 37, 0, 0, 5, 2, 0, 0, 0
    item_tail 8, 0, 0, 0, 0, 64, 0, 0

    item_header 96, 219, 802, 8000, 160, 15
    item_properties 8, 0, 1, 1, 75, 0, 163
    item_modifiers 34, 0, 5, 10, 2, 0, 0, 0
    item_tail 8, 0, 0, 0, 0, 65, 0, 0

    item_header 97, 220, 803, 0, 184, 11
    item_properties 8, 0, 1, 1, 3, 1, 161
    item_modifiers 36, 0, 0, 0, 2, 0, 0, 0
    item_tail 8, 0, 47, 0, 0, 66, 0, 0

    item_header 98, 221, 804, 0, 136, 19
    item_properties 8, 0, 1, 1, 3, 2, 161
    item_modifiers 38, 0, 0, 0, 2, 0, 0, 0
    item_tail 8, 0, 0, 0, 0, 67, 0, 0

    item_header 99, 222, 805, 0, 244, 1
    item_properties 8, 0, 1, 1, 3, 1, 163
    item_modifiers 33, 0, 0, 0, 2, 0, 0, 0
    item_tail 8, 0, 0, 0, 0, 68, 0, 0

    item_header 100, 223, 806, 0, 88, 27
    item_properties 8, 0, 1, 1, 3, 2, 163
    item_modifiers 37, 0, 0, 0, 2, 0, 0, 0
    item_tail 8, 0, 33, 38, 0, 69, 0, 0

    item_header 101, 224, 807, 0, 208, 7
    item_properties 8, 0, 1, 1, 131, 0, 161
    item_modifiers 32, 0, 0, 0, 2, 0, 0, 1
    item_tail 30, 0, 0, 0, 0, 70, 0, 0

    item_header 102, 225, 808, 0, 244, 1
    item_properties 8, 0, 1, 1, 3, 0, 161
    item_modifiers 45, 0, 0, 0, 2, 3, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 103, 226, 809, 0, 244, 1
    item_properties 8, 8, 1, 1, 3, 1, 162
    item_modifiers 41, 0, 0, 5, 2, 0, 0, 0
    item_tail 0, 0, 37, 0, 0, 0, 0, 0

    item_header 104, 227, 810, 0, 244, 1
    item_properties 8, 0, 1, 1, 3, 1, 162
    item_modifiers 49, 0, 0, 0, 2, 0, 0, 0
    item_tail 0, 0, 50, 0, 0, 0, 0, 0

    item_header 105, 228, 811, 0, 244, 1
    item_properties 8, 2, 1, 1, 3, 2, 162
    item_modifiers 43, 0, 8, 0, 2, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 106, 229, 812, 2000, 232, 3
    item_properties 9, 0, 1, 1, 107, 0, 134
    item_modifiers 31, 0, 0, 0, 0, 0, 0, 0
    item_tail 9, 0, 0, 0, 0, 71, 0, 0

    item_header 107, 230, 813, 5000, 196, 9
    item_properties 9, 4, 1, 1, 107, 0, 134
    item_modifiers 31, 0, 0, 0, 0, 0, 0, 0
    item_tail 9, 0, 0, 0, 0, 72, 0, 0

    item_header 108, 231, 814, 7000, 172, 13
    item_properties 9, 1, 1, 1, 107, 1, 136
    item_modifiers 33, 0, 0, 0, 0, 0, 0, 0
    item_tail 9, 0, 0, 0, 0, 73, 0, 0

    item_header 109, 232, 815, 8000, 160, 15
    item_properties 9, 0, 1, 1, 107, 0, 136
    item_modifiers 35, 5, 0, 0, 0, 0, 0, 0
    item_tail 9, 0, 0, 0, 0, 74, 0, 0

    item_header 110, 233, 816, 0, 184, 11
    item_properties 9, 0, 1, 1, 3, 1, 134
    item_modifiers 34, 0, 0, 0, 0, 0, 0, 0
    item_tail 9, 0, 0, 0, 0, 75, 0, 0

    item_header 111, 234, 817, 10000, 136, 19
    item_properties 9, 0, 1, 1, 107, 0, 134
    item_modifiers 37, 0, 0, 0, 0, 0, 0, 0
    item_tail 9, 0, 0, 0, 0, 76, 0, 0

    item_header 112, 235, 818, 12000, 112, 23
    item_properties 9, 0, 1, 1, 75, 1, 134
    item_modifiers 40, 0, 0, 5, 0, 0, 0, 0
    item_tail 9, 0, 0, 0, 0, 77, 0, 0

    item_header 113, 236, 819, 15000, 76, 29
    item_properties 9, 7, 1, 1, 75, 0, 134
    item_modifiers 39, 0, 0, 5, 0, 0, 0, 0
    item_tail 9, 0, 31, 0, 0, 78, 0, 0

    item_header 114, 237, 820, 0, 64, 31
    item_properties 9, 0, 1, 1, 3, 0, 134
    item_modifiers 42, 0, 0, 0, 0, 0, 0, 0
    item_tail 9, 0, 0, 0, 0, 79, 0, 0

    item_header 115, 238, 821, 25000, 244, 1
    item_properties 9, 0, 1, 1, 3, 1, 137
    item_modifiers 65, 0, 0, 0, 0, 0, 0, 0
    item_tail 9, 0, 0, 0, 0, 80, 0, 0

    item_header 116, 239, 822, 0, 244, 1
    item_properties 9, 7, 1, 1, 3, 0, 134
    item_modifiers 22, 0, 2, 0, 0, 0, 0, 0
    item_tail 9, 0, 35, 38, 0, 81, 0, 0

    item_header 117, 240, 823, 0, 208, 7
    item_properties 9, 0, 1, 1, 131, 0, 134
    item_modifiers 32, 0, 0, 0, 0, 0, 0, 1
    item_tail 30, 0, 0, 0, 0, 82, 0, 0

    item_header 118, 241, 824, 0, 244, 1
    item_properties 9, 0, 1, 1, 3, 2, 136
    item_modifiers 79, 0, 5, 0, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 119, 242, 825, 0, 244, 1
    item_properties 9, 0, 1, 1, 3, 0, 135
    item_modifiers 47, 0, 0, 0, 2, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 120, 243, 826, 0, 244, 1
    item_properties 9, 0, 1, 1, 3, 1, 137
    item_modifiers 55, 0, 0, 0, 0, 2, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 121, 244, 827, 500, 250, 0
    item_properties 10, 0, 1, 1, 123, 0, 138
    item_modifiers 19, 0, 0, 5, 0, 0, 0, 0
    item_tail 10, 0, 62, 0, 0, 83, 0, 0

    item_header 122, 245, 828, 800, 144, 1
    item_properties 10, 0, 1, 1, 123, 0, 142
    item_modifiers 21, 5, 0, 5, 0, 0, 0, 0
    item_tail 10, 0, 0, 0, 0, 84, 0, 0

    item_header 123, 246, 829, 1500, 238, 2
    item_properties 10, 6, 1, 1, 123, 1, 144
    item_modifiers 21, 0, 3, 5, 0, 0, 0, 0
    item_tail 10, 0, 0, 0, 0, 85, 0, 0

    item_header 124, 247, 830, 7000, 172, 13
    item_properties 10, 0, 1, 1, 75, 2, 144
    item_modifiers 29, 0, 0, 5, 0, 0, 0, 0
    item_tail 10, 0, 63, 0, 0, 86, 0, 0

    item_header 125, 248, 831, 3000, 220, 5
    item_properties 10, 0, 1, 1, 107, 1, 143
    item_modifiers 23, 0, 0, 5, 0, 0, 0, 0
    item_tail 10, 0, 0, 0, 0, 87, 0, 0

    item_header 126, 249, 832, 4000, 208, 7
    item_properties 10, 0, 1, 1, 107, 0, 141
    item_modifiers 23, 0, 0, 5, 0, 0, 0, 0
    item_tail 10, 0, 0, 0, 0, 88, 0, 0

    item_header 127, 250, 833, 0, 88, 2
    item_properties 10, 0, 1, 1, 3, 1, 139
    item_modifiers 29, 0, 0, 5, 0, 0, 0, 0
    item_tail 10, 0, 52, 0, 0, 89, 0, 0

    item_header 128, 251, 834, 0, 232, 3
    item_properties 10, 4, 1, 1, 3, 0, 143
    item_modifiers 28, 0, 0, 5, 0, 0, 0, 0
    item_tail 10, 0, 17, 0, 0, 90, 0, 0

    item_header 129, 252, 835, 9000, 148, 17
    item_properties 10, 0, 1, 1, 75, 1, 143
    item_modifiers 31, 5, 0, 5, 0, 0, 0, 0
    item_tail 10, 0, 0, 0, 0, 91, 0, 0

    item_header 130, 253, 836, 0, 160, 15
    item_properties 10, 0, 1, 1, 3, 0, 140
    item_modifiers 32, 0, 0, 5, 0, 2, 0, 0
    item_tail 10, 0, 0, 0, 0, 92, 0, 0

    item_header 131, 254, 837, 0, 112, 23
    item_properties 10, 7, 1, 1, 3, 2, 138
    item_modifiers 34, 0, 0, 10, 0, 0, 0, 0
    item_tail 10, 0, 0, 0, 0, 93, 0, 0

    item_header 132, 255, 838, 0, 208, 7
    item_properties 10, 0, 1, 1, 131, 0, 138
    item_modifiers 32, 0, 0, 5, 0, 0, 0, 1
    item_tail 30, 0, 0, 0, 0, 94, 0, 0

    item_header 133, 256, 839, 0, 244, 1
    item_properties 10, 0, 1, 1, 3, 2, 139
    item_modifiers 45, 6, 0, 5, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 134, 257, 840, 0, 244, 1
    item_properties 10, 0, 1, 1, 3, 1, 140
    item_modifiers 43, 0, 10, 15, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 135, 258, 841, 600, 44, 1
    item_properties 11, 0, 1, 1, 123, 2, 141
    item_modifiers 18, 0, 2, 0, 0, 0, 0, 0
    item_tail 11, 0, 0, 0, 0, 95, 0, 0

    item_header 136, 259, 842, 1500, 238, 2
    item_properties 11, 1, 1, 1, 107, 1, 143
    item_modifiers 21, 0, 2, 0, 0, 0, 0, 0
    item_tail 11, 0, 0, 0, 0, 96, 0, 0

    item_header 137, 260, 843, 1500, 238, 2
    item_properties 11, 6, 1, 1, 107, 2, 143
    item_modifiers 21, 0, 2, 0, 0, 0, 0, 0
    item_tail 11, 0, 0, 0, 0, 97, 0, 0

    item_header 138, 261, 844, 1500, 238, 2
    item_properties 11, 5, 1, 1, 107, 0, 143
    item_modifiers 21, 0, 2, 0, 0, 0, 0, 0
    item_tail 11, 0, 0, 0, 0, 98, 0, 0

    item_header 139, 262, 845, 2700, 70, 5
    item_properties 11, 3, 1, 1, 107, 2, 143
    item_modifiers 23, 0, 2, 0, 0, 0, 0, 0
    item_tail 11, 0, 15, 0, 0, 99, 0, 0

    item_header 140, 263, 846, 5000, 196, 9
    item_properties 11, 0, 1, 1, 75, 2, 144
    item_modifiers 25, 0, 5, 0, 0, 0, 0, 0
    item_tail 11, 0, 0, 0, 0, 100, 0, 0

    item_header 141, 264, 847, 0, 232, 3
    item_properties 11, 1, 1, 1, 3, 1, 143
    item_modifiers 27, 0, 2, 0, 0, 0, 0, 0
    item_tail 11, 0, 5, 0, 0, 101, 0, 0

    item_header 142, 265, 848, 0, 208, 7
    item_properties 11, 6, 1, 1, 3, 2, 144
    item_modifiers 27, 0, 2, 0, 0, 0, 0, 0
    item_tail 11, 0, 30, 0, 0, 102, 0, 0

    item_header 143, 266, 849, 0, 160, 15
    item_properties 11, 5, 1, 1, 3, 0, 143
    item_modifiers 27, 0, 2, 0, 0, 0, 0, 0
    item_tail 11, 0, 25, 0, 0, 103, 0, 0

    item_header 144, 267, 850, 0, 88, 27
    item_properties 11, 0, 1, 1, 3, 2, 143
    item_modifiers 29, 0, 5, 0, 0, 0, 0, 0
    item_tail 11, 0, 0, 0, 0, 104, 0, 0

    item_header 145, 268, 851, 20000, 244, 1
    item_properties 11, 0, 1, 1, 3, 1, 144
    item_modifiers 35, 5, 2, 5, 0, 2, 0, 0
    item_tail 11, 0, 10, 15, 20, 105, 0, 0

    item_header 146, 269, 852, 0, 208, 7
    item_properties 11, 0, 1, 1, 131, 0, 141
    item_modifiers 32, 0, 2, 0, 0, 0, 0, 1
    item_tail 30, 0, 0, 0, 0, 106, 0, 0

    item_header 147, 270, 853, 0, 244, 1
    item_properties 11, 8, 1, 1, 3, 1, 143
    item_modifiers 31, 0, 20, 0, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 148, 271, 854, 0, 244, 1
    item_properties 11, 0, 1, 1, 3, 2, 143
    item_modifiers 18, 2, 5, 2, 2, 2, 1, 1
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 149, 272, 855, 2000, 232, 3
    item_properties 12, 0, 1, 1, 107, 0, 175
    item_modifiers 31, 0, 0, 0, 0, 0, 0, 0
    item_tail 12, 0, 0, 0, 0, 107, 0, 0

    item_header 150, 273, 856, 3500, 214, 6
    item_properties 12, 0, 1, 1, 107, 1, 177
    item_modifiers 29, 0, 2, 5, 0, 0, 0, 0
    item_tail 12, 0, 0, 0, 0, 108, 0, 0

    item_header 151, 274, 857, 6000, 184, 11
    item_properties 12, 0, 1, 1, 107, 2, 174
    item_modifiers 33, 0, 3, 5, 0, 0, 0, 0
    item_tail 12, 0, 0, 0, 0, 109, 0, 0

    item_header 152, 275, 858, 9000, 148, 17
    item_properties 12, 0, 1, 1, 107, 0, 177
    item_modifiers 31, 0, 8, 8, 0, 0, 0, 0
    item_tail 12, 0, 0, 0, 0, 110, 0, 0

    item_header 153, 276, 859, 0, 20, 5
    item_properties 12, 0, 1, 1, 3, 0, 175
    item_modifiers 33, 0, 2, 5, 0, 0, 0, 0
    item_tail 12, 0, 0, 0, 0, 111, 0, 0

    item_header 154, 277, 860, 0, 208, 7
    item_properties 12, 3, 1, 1, 3, 2, 176
    item_modifiers 37, 0, 2, 5, 0, 0, 0, 0
    item_tail 12, 0, 11, 55, 0, 112, 0, 0

    item_header 155, 278, 861, 0, 112, 23
    item_properties 12, 0, 1, 1, 3, 1, 177
    item_modifiers 35, 0, 2, 10, 0, 0, 0, 0
    item_tail 12, 0, 0, 0, 0, 113, 0, 0

    item_header 156, 279, 862, 0, 64, 31
    item_properties 12, 1, 1, 1, 3, 1, 176
    item_modifiers 37, 0, 2, 5, 0, 0, 0, 0
    item_tail 12, 0, 5, 0, 0, 114, 0, 0

    item_header 157, 280, 863, 0, 16, 39
    item_properties 12, 0, 1, 1, 3, 1, 176
    item_modifiers 38, 0, 2, 5, 0, 0, 0, 0
    item_tail 12, 0, 0, 0, 0, 115, 0, 0

    item_header 158, 281, 864, 0, 244, 1
    item_properties 12, 7, 1, 1, 3, 2, 176
    item_modifiers 15, 0, 5, 5, 0, 0, 0, 0
    item_tail 12, 0, 35, 0, 0, 116, 0, 0

    item_header 159, 282, 865, 0, 208, 7
    item_properties 12, 0, 1, 1, 131, 0, 174
    item_modifiers 32, 0, 2, 5, 0, 0, 0, 1
    item_tail 30, 0, 0, 0, 0, 117, 0, 0

    item_header 160, 283, 866, 0, 244, 1
    item_properties 12, 0, 1, 1, 3, 0, 177
    item_modifiers 62, 0, 2, 5, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 161, 284, 867, 0, 244, 1
    item_properties 12, 0, 1, 1, 3, 2, 174
    item_modifiers 39, 0, 2, 5, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 162, 285, 868, 300, 150, 0
    item_properties 13, 0, 5, 2, 120, 2, 166
    item_modifiers 19, 0, 0, 0, 0, 0, 0, 0
    item_tail 13, 0, 0, 0, 0, 118, 0, 0

    item_header 163, 286, 869, 700, 94, 1
    item_properties 13, 0, 5, 2, 120, 0, 165
    item_modifiers 21, 0, 0, 0, 0, 0, 0, 0
    item_tail 13, 0, 0, 0, 0, 119, 0, 0

    item_header 164, 287, 870, 1500, 238, 2
    item_properties 13, 0, 5, 2, 120, 2, 166
    item_modifiers 25, 0, 0, 0, 0, 0, 0, 0
    item_tail 13, 0, 0, 0, 0, 120, 0, 0

    item_header 165, 288, 871, 4000, 208, 7
    item_properties 13, 0, 5, 2, 72, 2, 165
    item_modifiers 29, 0, 0, 0, 0, 0, 0, 0
    item_tail 13, 0, 0, 0, 0, 121, 0, 0

    item_header 166, 289, 872, 2500, 226, 4
    item_properties 13, 0, 6, 2, 104, 1, 165
    item_modifiers 23, 0, 0, 0, 0, 0, 0, 0
    item_tail 13, 0, 0, 0, 0, 122, 0, 0

    item_header 167, 290, 873, 0, 244, 1
    item_properties 13, 0, 7, 2, 0, 2, 167
    item_modifiers 27, 0, 0, 0, 0, 0, 0, 0
    item_tail 13, 0, 0, 0, 0, 123, 0, 0

    item_header 168, 291, 874, 0, 220, 5
    item_properties 13, 0, 5, 2, 0, 1, 166
    item_modifiers 33, 0, 0, 0, 0, 0, 0, 0
    item_tail 13, 0, 0, 0, 0, 124, 0, 0

    item_header 169, 292, 875, 0, 184, 11
    item_properties 13, 0, 5, 2, 0, 0, 166
    item_modifiers 35, 0, 0, 0, 0, 5, 0, 0
    item_tail 13, 0, 0, 0, 0, 125, 0, 0

    item_header 170, 293, 876, 0, 136, 19
    item_properties 13, 0, 6, 2, 0, 1, 166
    item_modifiers 42, 0, 0, 0, 0, 2, 0, 0
    item_tail 13, 0, 0, 0, 0, 126, 0, 0

    item_header 171, 294, 877, 0, 208, 7
    item_properties 13, 0, 5, 2, 128, 0, 164
    item_modifiers 32, 0, 0, 0, 0, 0, 0, 1
    item_tail 30, 0, 0, 0, 0, 127, 0, 0

    item_header 172, 295, 878, 0, 244, 1
    item_properties 13, 0, 5, 2, 0, 0, 166
    item_modifiers 45, 0, 0, 0, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 173, 296, 879, 0, 244, 1
    item_properties 13, 0, 5, 2, 0, 0, 167
    item_modifiers 55, 0, 0, 0, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 174, 297, 880, 3000, 220, 5
    item_properties 14, 2, 5, 2, 104, 0, 166
    item_modifiers 25, 0, 0, 0, 0, 2, 0, 0
    item_tail 14, 0, 0, 0, 0, 128, 0, 0

    item_header 175, 298, 881, 2000, 232, 3
    item_properties 14, 3, 5, 2, 104, 0, 167
    item_modifiers 23, 0, 0, 0, 0, 0, 0, 0
    item_tail 14, 0, 0, 0, 0, 129, 0, 0

    item_header 176, 299, 882, 5000, 196, 9
    item_properties 14, 0, 5, 2, 104, 2, 166
    item_modifiers 29, 0, 0, 0, 0, 0, 0, 0
    item_tail 14, 0, 0, 0, 0, 130, 0, 0

    item_header 177, 300, 883, 8000, 160, 15
    item_properties 14, 0, 5, 2, 72, 0, 166
    item_modifiers 31, 0, 0, 0, 0, 0, 0, 0
    item_tail 14, 0, 0, 0, 0, 131, 0, 0

    item_header 178, 301, 884, 40000, 244, 1
    item_properties 14, 0, 5, 2, 0, 2, 166
    item_modifiers 33, 0, 0, 0, 0, 0, 0, 0
    item_tail 14, 0, 0, 0, 0, 132, 0, 0

    item_header 179, 302, 885, 0, 208, 7
    item_properties 14, 2, 6, 2, 0, 0, 167
    item_modifiers 31, 0, 0, 0, 0, 2, 0, 0
    item_tail 14, 0, 0, 0, 0, 133, 0, 0

    item_header 180, 303, 886, 0, 136, 19
    item_properties 14, 8, 5, 2, 0, 1, 167
    item_modifiers 33, 0, 0, 0, 0, 0, 0, 0
    item_tail 14, 0, 0, 0, 0, 134, 0, 0

    item_header 181, 304, 887, 0, 88, 27
    item_properties 14, 6, 5, 2, 0, 1, 164
    item_modifiers 37, 5, 0, 0, 0, 0, 0, 0
    item_tail 14, 0, 0, 0, 0, 135, 0, 0

    item_header 182, 305, 888, 0, 16, 39
    item_properties 14, 0, 5, 2, 0, 0, 164
    item_modifiers 41, 0, 0, 0, 0, 2, 0, 0
    item_tail 14, 0, 0, 0, 0, 136, 0, 0

    item_header 183, 306, 889, 0, 244, 1
    item_properties 14, 8, 6, 2, 0, 1, 166
    item_modifiers 61, 0, 2, 0, 0, 0, 0, 0
    item_tail 14, 0, 0, 0, 0, 137, 0, 0

    item_header 184, 307, 890, 23000, 244, 1
    item_properties 14, 7, 5, 2, 0, 2, 166
    item_modifiers 15, 0, 0, 0, 0, 5, 0, 0
    item_tail 14, 0, 0, 0, 0, 138, 0, 0

    item_header 185, 308, 891, 0, 208, 7
    item_properties 14, 0, 5, 2, 128, 0, 164
    item_modifiers 32, 0, 0, 0, 0, 0, 0, 1
    item_tail 30, 0, 0, 0, 0, 139, 0, 0

    item_header 186, 309, 892, 0, 244, 1
    item_properties 14, 0, 7, 2, 0, 2, 166
    item_modifiers 39, 0, 0, 0, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 187, 310, 893, 0, 244, 1
    item_properties 14, 0, 7, 2, 0, 0, 164
    item_modifiers 51, 0, 0, 0, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 188, 311, 894, 0, 244, 1
    item_properties 14, 3, 7, 2, 0, 2, 164
    item_modifiers 42, 0, 0, 0, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 189, 312, 895, 3000, 220, 5
    item_properties 15, 0, 2, 1, 106, 0, 157
    item_modifiers 31, 0, 0, 0, 0, 0, 0, 0
    item_tail 15, 0, 0, 0, 0, 140, 0, 0

    item_header 190, 313, 896, 5000, 196, 9
    item_properties 15, 1, 2, 1, 106, 1, 159
    item_modifiers 33, 0, 0, 0, 0, 0, 0, 0
    item_tail 15, 0, 0, 0, 0, 141, 0, 0

    item_header 191, 314, 897, 14000, 88, 27
    item_properties 15, 6, 2, 1, 74, 2, 157
    item_modifiers 39, 0, 0, 0, 0, 0, 0, 0
    item_tail 15, 0, 0, 0, 0, 142, 0, 0

    item_header 192, 315, 898, 8000, 160, 15
    item_properties 15, 5, 2, 1, 106, 0, 159
    item_modifiers 35, 0, 0, 0, 0, 0, 0, 0
    item_tail 15, 0, 0, 0, 0, 143, 0, 0

    item_header 193, 316, 899, 10000, 136, 19
    item_properties 15, 0, 2, 1, 106, 0, 157
    item_modifiers 42, 0, 0, 0, 0, 0, 0, 1
    item_tail 15, 0, 0, 0, 0, 144, 0, 0

    item_header 194, 317, 900, 0, 184, 11
    item_properties 15, 0, 2, 1, 2, 0, 158
    item_modifiers 47, 0, 0, 0, 0, 0, 0, 1
    item_tail 15, 0, 0, 0, 0, 145, 0, 0

    item_header 195, 318, 901, 0, 112, 23
    item_properties 15, 0, 2, 1, 2, 1, 157
    item_modifiers 50, 0, 2, 0, 0, 0, 0, 1
    item_tail 15, 0, 0, 0, 0, 146, 0, 0

    item_header 196, 319, 902, 18000, 40, 35
    item_properties 15, 0, 2, 1, 74, 0, 160
    item_modifiers 45, 0, 0, 0, 0, 0, 0, 1
    item_tail 15, 0, 0, 0, 0, 147, 0, 0

    item_header 197, 320, 903, 0, 208, 7
    item_properties 15, 0, 2, 1, 130, 0, 157
    item_modifiers 32, 0, 0, 0, 0, 0, 0, 2
    item_tail 30, 0, 0, 0, 0, 148, 0, 0

    item_header 198, 321, 904, 0, 244, 1
    item_properties 15, 0, 2, 1, 2, 0, 157
    item_modifiers 55, 0, 0, 0, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 199, 322, 905, 0, 244, 1
    item_properties 15, 0, 2, 1, 2, 0, 157
    item_modifiers 51, 10, 0, 0, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 200, 323, 906, 0, 244, 1
    item_properties 15, 0, 2, 1, 2, 2, 157
    item_modifiers 53, 5, 2, 5, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 201, 324, 907, 400, 200, 0
    item_properties 16, 0, 1, 1, 122, 1, 146
    item_modifiers 22, 0, 0, 0, 0, 0, 0, 0
    item_tail 16, 0, 0, 0, 0, 149, 0, 0

    item_header 202, 325, 908, 1000, 244, 1
    item_properties 16, 0, 1, 1, 122, 0, 146
    item_modifiers 25, 0, 0, 1, 0, 0, 0, 0
    item_tail 16, 0, 51, 0, 0, 150, 0, 0

    item_header 203, 326, 909, 1700, 82, 3
    item_properties 16, 0, 1, 1, 122, 0, 148
    item_modifiers 25, 0, 0, 0, 0, 1, 0, 0
    item_tail 16, 0, 0, 0, 0, 151, 0, 0

    item_header 204, 327, 910, 3000, 220, 5
    item_properties 16, 0, 1, 1, 106, 1, 149
    item_modifiers 31, 2, 0, 0, 0, 0, 0, 0
    item_tail 16, 0, 0, 0, 0, 152, 0, 0

    item_header 205, 328, 911, 5000, 196, 9
    item_properties 16, 3, 1, 1, 106, 0, 147
    item_modifiers 31, 3, 0, 0, 0, 0, 0, 0
    item_tail 16, 0, 11, 0, 0, 153, 0, 0

    item_header 206, 329, 912, 7000, 172, 13
    item_properties 16, 8, 1, 1, 106, 1, 151
    item_modifiers 33, 0, 0, 2, 0, 0, 0, 0
    item_tail 16, 0, 0, 0, 0, 154, 0, 0

    item_header 207, 330, 913, 10000, 136, 19
    item_properties 16, 0, 1, 1, 106, 1, 150
    item_modifiers 35, 0, 0, 0, 0, 1, 0, 0
    item_tail 16, 0, 44, 0, 0, 155, 0, 0

    item_header 208, 331, 914, 0, 232, 3
    item_properties 16, 0, 1, 1, 2, 0, 182
    item_modifiers 29, 0, 2, 0, 0, 0, 0, 0
    item_tail 16, 0, 0, 0, 0, 156, 0, 0

    item_header 209, 332, 915, 0, 160, 15
    item_properties 16, 0, 1, 1, 2, 1, 152
    item_modifiers 32, 0, 0, 2, 0, 0, 0, 0
    item_tail 16, 0, 55, 0, 0, 157, 0, 0

    item_header 210, 333, 916, 0, 64, 31
    item_properties 16, 7, 1, 1, 2, 2, 147
    item_modifiers 39, 0, 0, 0, 0, 1, 0, 0
    item_tail 16, 0, 50, 0, 0, 158, 0, 0

    item_header 211, 334, 917, 0, 16, 39
    item_properties 16, 0, 1, 1, 2, 1, 181
    item_modifiers 22, 0, 0, 0, 0, 1, 0, 0
    item_tail 16, 0, 61, 0, 0, 159, 0, 0

    item_header 212, 335, 918, 0, 208, 7
    item_properties 16, 0, 1, 1, 130, 0, 145
    item_modifiers 32, 0, 0, 0, 0, 0, 0, 1
    item_tail 30, 0, 0, 0, 0, 160, 0, 0

    item_header 213, 336, 919, 0, 244, 1
    item_properties 16, 8, 1, 1, 2, 1, 183
    item_modifiers 45, 0, 0, 0, 0, 0, 0, 0
    item_tail 0, 0, 54, 0, 0, 0, 0, 0

    item_header 214, 337, 920, 0, 244, 1
    item_properties 16, 8, 1, 1, 2, 1, 184
    item_modifiers 47, 0, 0, 0, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 215, 338, 921, 500, 250, 0
    item_properties 17, 0, 1, 1, 122, 0, 153
    item_modifiers 29, 0, 0, 0, 0, 1, 0, 0
    item_tail 17, 0, 0, 0, 0, 161, 0, 0

    item_header 216, 339, 922, 1000, 244, 1
    item_properties 17, 1, 1, 1, 122, 1, 154
    item_modifiers 31, 0, 0, 0, 0, 1, 0, 0
    item_tail 17, 0, 0, 0, 0, 162, 0, 0

    item_header 217, 340, 923, 2000, 232, 3
    item_properties 17, 0, 1, 1, 106, 0, 155
    item_modifiers 35, 0, 0, 0, 0, 1, 0, 0
    item_tail 17, 0, 0, 0, 0, 163, 0, 0

    item_header 218, 341, 924, 3200, 64, 6
    item_properties 17, 0, 1, 1, 106, 0, 154
    item_modifiers 39, 0, 0, 0, 0, 1, 0, 0
    item_tail 17, 0, 0, 0, 0, 164, 0, 0

    item_header 219, 342, 925, 5700, 34, 11
    item_properties 17, 0, 1, 1, 106, 2, 153
    item_modifiers 42, 0, 0, 0, 0, 1, 0, 0
    item_tail 17, 0, 0, 0, 0, 165, 0, 0

    item_header 220, 343, 926, 0, 220, 5
    item_properties 17, 0, 1, 1, 2, 1, 153
    item_modifiers 35, 0, 0, 0, 2, 1, 0, 0
    item_tail 17, 0, 0, 0, 0, 166, 0, 0

    item_header 221, 344, 927, 0, 208, 7
    item_properties 17, 0, 1, 1, 2, 2, 154
    item_modifiers 37, 2, 0, 0, 0, 2, 0, 0
    item_tail 17, 0, 0, 0, 0, 167, 0, 0

    item_header 222, 345, 928, 0, 160, 15
    item_properties 17, 5, 1, 1, 2, 0, 153
    item_modifiers 39, 0, 0, 0, 0, 1, 0, 0
    item_tail 17, 0, 0, 0, 0, 168, 0, 0

    item_header 223, 346, 929, 0, 112, 23
    item_properties 17, 7, 1, 1, 2, 0, 153
    item_modifiers 39, 0, 3, 0, 1, 5, 0, 0
    item_tail 17, 0, 0, 0, 0, 169, 0, 0

    item_header 224, 347, 930, 0, 64, 31
    item_properties 17, 6, 1, 1, 2, 2, 154
    item_modifiers 41, 0, 0, 0, 0, 2, 0, 0
    item_tail 17, 0, 0, 0, 0, 170, 0, 0

    item_header 225, 348, 931, 0, 16, 39
    item_properties 17, 8, 1, 1, 2, 1, 154
    item_modifiers 43, 0, 0, 0, 0, 1, 0, 0
    item_tail 17, 0, 0, 0, 0, 171, 0, 0

    item_header 226, 349, 932, 0, 208, 7
    item_properties 17, 0, 1, 1, 130, 0, 153
    item_modifiers 32, 0, 0, 0, 0, 1, 0, 1
    item_tail 30, 0, 0, 0, 0, 172, 0, 0

    item_header 227, 350, 933, 0, 244, 1
    item_properties 17, 0, 1, 1, 2, 1, 154
    item_modifiers 59, 0, 0, 0, 0, 1, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 228, 351, 934, 0, 244, 1
    item_properties 17, 0, 1, 1, 2, 1, 156
    item_modifiers 52, 0, 0, 0, 0, 1, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 229, 352, 935, 0, 188, 2
    item_properties 18, 0, 1, 1, 2, 2, 178
    item_modifiers 32, 0, 2, 0, 0, 1, 0, 0
    item_tail 18, 0, 23, 0, 0, 173, 0, 0

    item_header 230, 353, 936, 0, 232, 3
    item_properties 18, 0, 1, 1, 2, 0, 178
    item_modifiers 34, 10, 2, 0, 0, 0, 0, 0
    item_tail 18, 0, 28, 0, 0, 174, 0, 0

    item_header 231, 354, 937, 0, 176, 4
    item_properties 18, 1, 1, 1, 2, 1, 178
    item_modifiers 36, 0, 2, 0, 0, 0, 0, 0
    item_tail 18, 0, 3, 0, 0, 175, 0, 0

    item_header 232, 355, 938, 0, 208, 7
    item_properties 18, 0, 1, 1, 2, 2, 180
    item_modifiers 43, 5, 2, 0, 0, 0, 0, 0
    item_tail 18, 0, 11, 0, 0, 176, 0, 0

    item_header 233, 356, 939, 0, 220, 5
    item_properties 18, 4, 1, 1, 2, 1, 180
    item_modifiers 41, 0, 2, 0, 0, 0, 0, 0
    item_tail 18, 0, 51, 0, 0, 177, 0, 0

    item_header 234, 357, 940, 0, 20, 5
    item_properties 18, 3, 1, 1, 2, 2, 179
    item_modifiers 39, 2, 2, 0, 0, 0, 0, 0
    item_tail 18, 0, 48, 0, 0, 178, 0, 0

    item_header 235, 358, 941, 0, 208, 7
    item_properties 18, 0, 1, 1, 2, 1, 178
    item_modifiers 39, 0, 2, 0, 0, 2, 0, 0
    item_tail 18, 0, 47, 0, 0, 179, 0, 0

    item_header 236, 359, 942, 0, 184, 11
    item_properties 18, 0, 1, 1, 2, 0, 180
    item_modifiers 47, 0, 2, 2, 0, 0, 0, 0
    item_tail 18, 0, 55, 0, 0, 180, 0, 0

    item_header 237, 360, 943, 0, 120, 5
    item_properties 18, 8, 1, 1, 2, 0, 178
    item_modifiers 45, 0, 2, 0, 0, 0, 0, 0
    item_tail 18, 0, 50, 0, 0, 181, 0, 0

    item_header 238, 361, 944, 0, 208, 7
    item_properties 18, 0, 1, 1, 130, 0, 179
    item_modifiers 32, 0, 2, 0, 0, 0, 0, 1
    item_tail 30, 0, 0, 0, 0, 182, 0, 0

    item_header 239, 362, 945, 0, 244, 1
    item_properties 18, 0, 1, 1, 2, 1, 180
    item_modifiers 49, 0, 2, 0, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 240, 363, 946, 0, 244, 1
    item_properties 18, 0, 1, 1, 2, 1, 178
    item_modifiers 67, 0, 2, 0, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 241, 364, 947, 2000, 232, 3
    item_properties 19, 0, 8, 1, 122, 2, 173
    item_modifiers 27, 0, 0, 0, 0, 0, 0, 0
    item_tail 19, 0, 0, 0, 0, 183, 0, 0

    item_header 242, 365, 948, 3000, 220, 5
    item_properties 19, 0, 7, 1, 106, 0, 171
    item_modifiers 31, 0, 0, 0, 0, 0, 0, 0
    item_tail 19, 0, 0, 0, 0, 184, 0, 0

    item_header 243, 366, 949, 5000, 196, 9
    item_properties 19, 0, 8, 1, 74, 0, 170
    item_modifiers 31, 0, 0, 0, 0, 0, 0, 0
    item_tail 19, 0, 0, 0, 0, 185, 0, 0

    item_header 244, 367, 950, 8000, 160, 15
    item_properties 19, 0, 8, 1, 74, 1, 169
    item_modifiers 33, 0, 0, 0, 0, 0, 0, 0
    item_tail 19, 0, 0, 0, 0, 186, 0, 0

    item_header 245, 368, 951, 0, 176, 4
    item_properties 19, 0, 8, 1, 2, 0, 172
    item_modifiers 31, 0, 0, 0, 0, 0, 0, 0
    item_tail 19, 0, 0, 0, 0, 187, 0, 0

    item_header 246, 369, 952, 0, 208, 7
    item_properties 19, 0, 8, 1, 2, 1, 171
    item_modifiers 33, 0, 0, 0, 0, 0, 0, 0
    item_tail 19, 0, 0, 0, 0, 188, 0, 0

    item_header 247, 370, 953, 0, 184, 11
    item_properties 19, 0, 8, 1, 2, 0, 168
    item_modifiers 37, 0, 0, 0, 0, 0, 0, 0
    item_tail 19, 0, 0, 0, 0, 189, 0, 0

    item_header 248, 371, 954, 0, 112, 23
    item_properties 19, 0, 8, 1, 2, 0, 170
    item_modifiers 39, 0, 0, 0, 0, 0, 0, 0
    item_tail 19, 0, 0, 0, 0, 190, 0, 0

    item_header 249, 372, 955, 0, 64, 31
    item_properties 19, 0, 9, 1, 2, 1, 168
    item_modifiers 41, 0, 0, 0, 0, 0, 0, 0
    item_tail 19, 0, 0, 0, 0, 191, 0, 0

    item_header 250, 373, 956, 0, 208, 7
    item_properties 19, 0, 8, 1, 130, 0, 171
    item_modifiers 32, 0, 0, 0, 0, 0, 0, 1
    item_tail 30, 0, 0, 0, 0, 192, 0, 0

    item_header 251, 374, 957, 0, 244, 1
    item_properties 19, 0, 8, 1, 2, 0, 170
    item_modifiers 47, 0, 0, 0, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 252, 375, 958, 0, 244, 1
    item_properties 19, 0, 8, 1, 2, 2, 170
    item_modifiers 59, 0, 0, 0, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 253, 376, 959, 400, 200, 0
    item_properties 20, 0, 0, 1, 122, 0, 0
    item_modifiers 0, 0, 0, 2, 0, 4, 0, 0
    item_tail 20, 0, 0, 0, 0, 193, 0, 0

    item_header 254, 377, 960, 1000, 244, 1
    item_properties 20, 0, 0, 1, 106, 0, 0
    item_modifiers 0, 0, 0, 4, 0, 5, 0, 0
    item_tail 20, 0, 0, 0, 0, 222, 0, 0

    item_header 255, 378, 961, 2000, 232, 3
    item_properties 20, 0, 0, 1, 74, 0, 0
    item_modifiers 0, 2, 0, 6, 0, 7, 0, 0
    item_tail 20, 0, 0, 0, 0, 223, 0, 0

    item_header 256, 379, 962, 0, 220, 5
    item_properties 20, 5, 0, 1, 2, 0, 0
    item_modifiers 0, 0, 0, 6, 0, 9, 0, 0
    item_tail 20, 0, 21, 3, 29, 0, 0, 0

    item_header 257, 380, 963, 0, 220, 5
    item_properties 20, 1, 0, 1, 2, 0, 0
    item_modifiers 0, 0, 0, 6, 0, 9, 0, 0
    item_tail 20, 0, 1, 23, 19, 0, 0, 0

    item_header 258, 381, 964, 0, 184, 11
    item_properties 20, 7, 0, 1, 2, 0, 0
    item_modifiers 0, 5, 0, 5, 0, 10, 0, 0
    item_tail 20, 0, 52, 0, 0, 0, 0, 0

    item_header 259, 382, 965, 0, 244, 1
    item_properties 20, 0, 0, 1, 2, 0, 0
    item_modifiers 0, 10, 0, 7, 0, 10, 0, 0
    item_tail 20, 0, 0, 0, 0, 0, 0, 0

    item_header 260, 383, 966, 0, 112, 23
    item_properties 20, 7, 0, 1, 2, 0, 0
    item_modifiers 0, 5, 0, 5, 0, 10, 0, 0
    item_tail 20, 0, 58, 0, 0, 0, 0, 0

    item_header 261, 384, 967, 0, 244, 1
    item_properties 20, 0, 0, 1, 2, 0, 0
    item_modifiers 0, 0, 0, 10, 0, 7, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 262, 385, 968, 0, 244, 1
    item_properties 20, 0, 0, 1, 2, 0, 0
    item_modifiers 0, 0, 0, 0, 0, 10, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 263, 386, 969, 0, 244, 1
    item_properties 20, 0, 0, 1, 2, 0, 0
    item_modifiers 0, 0, 0, 5, 0, 15, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 264, 387, 970, 0, 244, 1
    item_properties 20, 0, 0, 1, 2, 0, 0
    item_modifiers 0, 5, 0, 10, 0, 10, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 265, 388, 971, 500, 250, 0
    item_properties 21, 0, 0, 3, 120, 0, 0
    item_modifiers 0, 4, 0, 2, 0, 0, 0, 0
    item_tail 21, 0, 0, 0, 0, 0, 0, 0

    item_header 266, 389, 972, 1200, 88, 2
    item_properties 21, 0, 0, 3, 104, 0, 0
    item_modifiers 0, 5, 0, 3, 0, 0, 0, 0
    item_tail 21, 0, 0, 0, 0, 0, 0, 0

    item_header 267, 390, 973, 2500, 226, 4
    item_properties 21, 0, 0, 3, 72, 0, 0
    item_modifiers 0, 7, 0, 3, 0, 0, 0, 0
    item_tail 21, 0, 0, 0, 0, 0, 0, 0

    item_header 268, 391, 974, 0, 208, 7
    item_properties 21, 0, 0, 3, 0, 0, 0
    item_modifiers 0, 9, 0, 4, 0, 0, 0, 0
    item_tail 21, 0, 0, 0, 0, 0, 0, 0

    item_header 269, 392, 975, 0, 136, 19
    item_properties 21, 0, 0, 3, 0, 0, 0
    item_modifiers 0, 11, 0, 5, 0, 0, 0, 0
    item_tail 21, 0, 0, 0, 0, 0, 0, 0

    item_header 270, 393, 976, 0, 244, 1
    item_properties 21, 0, 0, 3, 0, 0, 0
    item_modifiers 0, 15, 0, 6, 0, 0, 0, 0
    item_tail 21, 0, 0, 0, 0, 0, 0, 0

    item_header 271, 394, 977, 0, 244, 1
    item_properties 21, 0, 0, 3, 0, 0, 0
    item_modifiers 0, 13, 0, 4, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 272, 395, 978, 0, 244, 1
    item_properties 21, 0, 0, 3, 0, 0, 0
    item_modifiers 0, 12, 0, 8, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 273, 396, 979, 0, 244, 1
    item_properties 21, 0, 0, 3, 0, 0, 0
    item_modifiers 0, 16, 0, 6, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 274, 397, 980, 0, 232, 3
    item_properties 22, 0, 0, 3, 0, 0, 0
    item_modifiers 0, 2, 0, 5, 0, 0, 0, 0
    item_tail 22, 0, 57, 0, 0, 0, 0, 0

    item_header 275, 398, 981, 0, 208, 7
    item_properties 22, 0, 0, 3, 0, 0, 0
    item_modifiers 0, 2, 0, 5, 0, 0, 0, 0
    item_tail 22, 0, 58, 0, 0, 0, 0, 0

    item_header 276, 399, 982, 0, 32, 78
    item_properties 22, 0, 0, 3, 0, 0, 0
    item_modifiers 0, 2, 0, 5, 0, 0, 0, 0
    item_tail 22, 0, 56, 0, 0, 0, 0, 0

    item_header 277, 400, 983, 350, 175, 0
    item_properties 23, 0, 0, 3, 120, 0, 0
    item_modifiers 0, 2, 0, 4, 0, 0, 0, 0
    item_tail 22, 0, 0, 0, 0, 0, 0, 0

    item_header 278, 401, 984, 1800, 132, 3
    item_properties 23, 0, 0, 3, 72, 0, 0
    item_modifiers 0, 3, 0, 3, 0, 0, 0, 0
    item_tail 22, 0, 0, 0, 0, 0, 0, 0

    item_header 279, 402, 985, 800, 144, 1
    item_properties 23, 0, 0, 3, 120, 0, 0
    item_modifiers 0, 2, 0, 2, 0, 2, 0, 0
    item_tail 22, 0, 0, 0, 0, 224, 0, 0

    item_header 280, 403, 986, 3000, 220, 5
    item_properties 23, 0, 0, 3, 72, 0, 0
    item_modifiers 5, 6, 0, 2, 0, 0, 0, 0
    item_tail 22, 0, 0, 0, 0, 0, 0, 0

    item_header 281, 404, 987, 0, 232, 3
    item_properties 23, 0, 0, 3, 0, 0, 0
    item_modifiers 0, 3, 1, 10, 0, 0, 0, 0
    item_tail 22, 0, 0, 0, 0, 0, 0, 0

    item_header 282, 405, 988, 0, 208, 7
    item_properties 23, 0, 0, 3, 0, 0, 0
    item_modifiers 0, 4, 2, 12, 0, 0, 0, 0
    item_tail 22, 0, 54, 0, 0, 0, 0, 0

    item_header 283, 406, 989, 0, 184, 11
    item_properties 23, 0, 0, 3, 0, 0, 0
    item_modifiers 0, 8, 0, 6, 0, 7, 0, 0
    item_tail 22, 0, 46, 45, 0, 0, 0, 0

    item_header 284, 407, 990, 0, 160, 15
    item_properties 23, 0, 0, 3, 0, 0, 0
    item_modifiers 0, 4, 4, 16, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 285, 408, 991, 0, 160, 15
    item_properties 23, 0, 0, 3, 0, 0, 0
    item_modifiers 0, 4, 0, 14, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 286, 409, 992, 0, 244, 1
    item_properties 23, 0, 0, 3, 0, 0, 0
    item_modifiers 0, 2, 5, 2, 2, 5, 1, 1
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 287, 410, 993, 0, 244, 1
    item_properties 23, 0, 0, 3, 0, 0, 0
    item_modifiers 0, 8, 0, 20, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 288, 411, 994, 400, 200, 0
    item_properties 24, 0, 0, 4, 120, 0, 0
    item_modifiers 0, 28, 0, 2, 0, 0, 0, 0
    item_tail 23, 0, 0, 0, 0, 0, 0, 0

    item_header 289, 412, 995, 1000, 244, 1
    item_properties 24, 0, 0, 4, 120, 0, 0
    item_modifiers 0, 30, 0, 6, 0, 0, 0, 0
    item_tail 23, 0, 0, 0, 0, 0, 0, 0

    item_header 290, 413, 996, 1500, 238, 2
    item_properties 24, 0, 0, 4, 104, 0, 0
    item_modifiers 0, 34, 0, 3, 0, 0, 0, 0
    item_tail 23, 0, 0, 0, 0, 0, 0, 0

    item_header 291, 414, 997, 3000, 220, 5
    item_properties 24, 0, 0, 4, 72, 0, 0
    item_modifiers 0, 38, 0, 3, 0, 0, 0, 0
    item_tail 23, 0, 0, 0, 0, 0, 0, 0

    item_header 292, 415, 998, 6000, 184, 11
    item_properties 24, 0, 0, 4, 72, 0, 0
    item_modifiers 0, 42, 0, 6, 0, 0, 0, 0
    item_tail 23, 0, 0, 0, 0, 194, 0, 0

    item_header 293, 416, 999, 0, 208, 7
    item_properties 24, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 40, 0, 3, 0, 0, 0, 0
    item_tail 23, 0, 0, 0, 0, 195, 0, 0

    item_header 294, 417, 1000, 0, 184, 11
    item_properties 24, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 42, 0, 3, 0, 0, 0, 0
    item_tail 23, 0, 0, 0, 0, 0, 0, 0

    item_header 295, 418, 1001, 0, 160, 15
    item_properties 24, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 38, 0, 8, 0, 0, 0, 0
    item_tail 23, 0, 0, 0, 0, 196, 0, 0

    item_header 296, 419, 1002, 0, 112, 23
    item_properties 24, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 36, 0, 8, 0, 0, 0, 0
    item_tail 23, 0, 65, 0, 0, 0, 0, 0

    item_header 297, 420, 1003, 0, 64, 31
    item_properties 24, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 40, 0, 8, 0, 0, 0, 0
    item_tail 23, 0, 3, 0, 0, 0, 0, 0

    item_header 298, 421, 1004, 0, 244, 1
    item_properties 24, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 46, 0, 12, 0, 0, 0, 0
    item_tail 23, 0, 0, 0, 0, 197, 0, 0

    item_header 299, 422, 1005, 0, 16, 39
    item_properties 24, 0, 0, 4, 0, 0, 0
    item_modifiers 3, 46, 0, 10, 0, 0, 0, 0
    item_tail 23, 0, 0, 0, 0, 0, 0, 0

    item_header 300, 423, 1006, 0, 244, 1
    item_properties 24, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 58, 0, 3, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 301, 424, 1007, 0, 244, 1
    item_properties 24, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 52, 0, 16, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 302, 425, 1008, 0, 244, 1
    item_properties 24, 0, 0, 4, 0, 0, 0
    item_modifiers 5, 28, 5, 2, 2, 5, 1, 1
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 303, 426, 1009, 300, 150, 0
    item_properties 25, 0, 0, 4, 120, 0, 0
    item_modifiers 0, 18, 0, 4, 0, 0, 0, 0
    item_tail 24, 0, 0, 0, 0, 0, 0, 0

    item_header 304, 427, 1010, 900, 194, 1
    item_properties 25, 0, 0, 4, 120, 0, 0
    item_modifiers 0, 28, 0, 4, 0, 0, 0, 0
    item_tail 24, 0, 0, 0, 0, 198, 0, 0

    item_header 305, 428, 1011, 1400, 188, 2
    item_properties 25, 0, 0, 4, 104, 0, 0
    item_modifiers 0, 30, 0, 3, 0, 0, 0, 0
    item_tail 24, 0, 0, 0, 0, 199, 0, 0

    item_header 306, 429, 1012, 2500, 226, 4
    item_properties 25, 0, 0, 4, 104, 0, 0
    item_modifiers 0, 34, 0, 6, 0, 0, 0, 0
    item_tail 24, 0, 0, 0, 0, 200, 0, 0

    item_header 307, 430, 1013, 3700, 58, 7
    item_properties 25, 0, 0, 4, 72, 0, 0
    item_modifiers 0, 37, 0, 6, 0, 0, 0, 0
    item_tail 24, 0, 0, 0, 0, 201, 0, 0

    item_header 308, 431, 1014, 6000, 184, 11
    item_properties 25, 0, 0, 4, 72, 0, 0
    item_modifiers 0, 34, 0, 8, 0, 0, 0, 0
    item_tail 24, 0, 50, 0, 0, 202, 0, 0

    item_header 309, 432, 1015, 7000, 172, 13
    item_properties 25, 0, 0, 4, 72, 0, 0
    item_modifiers 2, 34, 0, 10, 0, 0, 0, 0
    item_tail 24, 0, 0, 0, 0, 203, 0, 0

    item_header 310, 433, 1016, 0, 244, 1
    item_properties 25, 3, 0, 4, 0, 0, 0
    item_modifiers 0, 24, 0, 12, 0, 0, 0, 0
    item_tail 24, 0, 11, 0, 0, 204, 0, 0

    item_header 311, 434, 1017, 0, 220, 5
    item_properties 25, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 28, 0, 14, 0, 0, 0, 0
    item_tail 24, 0, 37, 0, 0, 0, 0, 0

    item_header 312, 435, 1018, 0, 196, 9
    item_properties 25, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 30, 0, 6, 1, 2, 0, 0
    item_tail 24, 0, 0, 0, 0, 205, 0, 0

    item_header 313, 436, 1019, 0, 184, 11
    item_properties 25, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 32, 0, 3, 2, 1, 0, 0
    item_tail 24, 0, 42, 0, 0, 206, 0, 0

    item_header 314, 437, 1020, 0, 160, 15
    item_properties 25, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 35, 0, 10, 0, 0, 0, 0
    item_tail 24, 0, 53, 0, 0, 207, 0, 0

    item_header 315, 438, 1021, 0, 136, 19
    item_properties 25, 7, 0, 4, 0, 0, 0
    item_modifiers 0, 32, 0, 16, 0, 0, 0, 0
    item_tail 24, 0, 0, 0, 0, 208, 0, 0

    item_header 316, 439, 1022, 0, 88, 27
    item_properties 25, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 28, 0, 16, 0, 0, 0, 0
    item_tail 24, 0, 27, 0, 0, 0, 0, 0

    item_header 317, 440, 1023, 0, 16, 39
    item_properties 25, 8, 0, 4, 0, 0, 0
    item_modifiers 0, 42, 0, 8, 0, 0, 0, 0
    item_tail 24, 0, 36, 0, 0, 0, 0, 0

    item_header 318, 441, 1024, 0, 244, 1
    item_properties 25, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 34, 0, 24, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 319, 442, 1025, 0, 244, 1
    item_properties 25, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 28, 0, 16, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 320, 443, 1026, 0, 244, 1
    item_properties 25, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 26, 0, 18, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 321, 444, 1027, 0, 244, 1
    item_properties 25, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 38, 0, 28, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 322, 445, 1028, 30000, 244, 1
    item_properties 25, 0, 0, 4, 0, 0, 0
    item_modifiers 2, 36, 2, 16, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 323, 446, 1029, 400, 200, 0
    item_properties 26, 0, 0, 4, 120, 0, 0
    item_modifiers 0, 15, 0, 22, 0, 0, 0, 0
    item_tail 25, 0, 0, 0, 0, 0, 0, 0

    item_header 324, 447, 1030, 1000, 244, 1
    item_properties 26, 0, 0, 4, 104, 0, 0
    item_modifiers 0, 15, 0, 28, 0, 0, 0, 0
    item_tail 25, 0, 0, 0, 0, 0, 0, 0

    item_header 325, 448, 1031, 3000, 220, 5
    item_properties 26, 0, 0, 4, 72, 0, 0
    item_modifiers 0, 15, 0, 30, 0, 0, 0, 0
    item_tail 25, 0, 0, 0, 0, 0, 0, 0

    item_header 326, 449, 1032, 5000, 196, 9
    item_properties 26, 7, 0, 4, 72, 0, 0
    item_modifiers 0, 19, 0, 30, 0, 0, 0, 0
    item_tail 25, 0, 53, 31, 0, 0, 0, 0

    item_header 327, 450, 1033, 0, 232, 3
    item_properties 26, 1, 0, 4, 0, 0, 0
    item_modifiers 0, 19, 0, 30, 0, 0, 0, 0
    item_tail 25, 0, 1, 0, 0, 0, 0, 0

    item_header 328, 451, 1034, 0, 232, 3
    item_properties 26, 6, 0, 4, 0, 0, 0
    item_modifiers 0, 19, 0, 30, 0, 0, 0, 0
    item_tail 25, 0, 26, 0, 0, 0, 0, 0

    item_header 329, 452, 1035, 0, 232, 3
    item_properties 26, 5, 0, 4, 0, 0, 0
    item_modifiers 0, 19, 0, 30, 0, 0, 0, 0
    item_tail 25, 0, 21, 0, 0, 0, 0, 0

    item_header 330, 453, 1036, 0, 184, 11
    item_properties 26, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 21, 0, 38, 0, 0, 0, 0
    item_tail 25, 0, 3, 28, 23, 210, 0, 0

    item_header 331, 454, 1037, 0, 184, 11
    item_properties 26, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 21, 2, 36, 0, 0, 0, 0
    item_tail 25, 0, 5, 30, 25, 211, 0, 0

    item_header 332, 455, 1038, 0, 160, 15
    item_properties 26, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 25, 0, 40, 0, 0, 0, 0
    item_tail 25, 0, 0, 0, 0, 212, 0, 0

    item_header 333, 456, 1039, 0, 88, 27
    item_properties 26, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 28, 0, 42, 0, 0, 0, 0
    item_tail 25, 0, 0, 0, 0, 213, 0, 0

    item_header 334, 457, 1040, 0, 16, 39
    item_properties 26, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 30, 0, 38, 0, 0, 0, 0
    item_tail 25, 0, 0, 0, 0, 0, 0, 0

    item_header 335, 458, 1041, 0, 160, 15
    item_properties 26, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 22, 0, 31, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 336, 459, 1042, 0, 244, 1
    item_properties 26, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 24, 0, 52, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 209, 0, 0

    item_header 337, 460, 1043, 0, 64, 31
    item_properties 26, 0, 0, 4, 0, 0, 0
    item_modifiers 0, 24, 6, 36, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 338, 461, 1044, 0, 244, 1
    item_properties 26, 8, 0, 4, 0, 0, 0
    item_modifiers 0, 32, 0, 36, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 339, 462, 1045, 1000, 244, 1
    item_properties 27, 0, 0, 5, 120, 0, 0
    item_modifiers 0, 7, 0, 0, 0, 0, 0, 0
    item_tail 26, 0, 0, 0, 0, 0, 0, 0

    item_header 340, 463, 1046, 1500, 238, 2
    item_properties 27, 0, 0, 5, 72, 0, 0
    item_modifiers 0, 4, 0, 0, 0, 0, 0, 1
    item_tail 26, 0, 0, 0, 0, 0, 0, 0

    item_header 341, 464, 1047, 2000, 232, 3
    item_properties 27, 0, 0, 5, 72, 0, 0
    item_modifiers 0, 2, 0, 0, 0, 0, 1, 0
    item_tail 26, 0, 0, 0, 0, 0, 0, 0

    item_header 342, 465, 1048, 0, 208, 7
    item_properties 27, 0, 0, 5, 0, 0, 0
    item_modifiers 0, 3, 0, 5, 0, 2, 0, 0
    item_tail 26, 0, 0, 0, 0, 0, 0, 0

    item_header 343, 466, 1049, 0, 184, 11
    item_properties 27, 0, 0, 5, 0, 0, 0
    item_modifiers 0, 3, 0, 0, 0, 0, 0, 0
    item_tail 26, 0, 0, 0, 0, 218, 0, 0

    item_header 344, 467, 1050, 0, 160, 15
    item_properties 27, 0, 0, 5, 0, 0, 0
    item_modifiers 0, 3, 0, 0, 0, 0, 0, 2
    item_tail 26, 0, 0, 0, 0, 0, 0, 0

    item_header 345, 468, 1051, 0, 148, 17
    item_properties 27, 0, 0, 5, 0, 0, 0
    item_modifiers 0, 3, 0, 0, 2, 0, 0, 0
    item_tail 26, 0, 0, 0, 0, 220, 0, 0

    item_header 346, 469, 1052, 0, 112, 23
    item_properties 27, 0, 0, 5, 0, 0, 0
    item_modifiers 0, 3, 0, 2, 0, 0, 0, 0
    item_tail 26, 0, 0, 0, 0, 221, 0, 0

    item_header 347, 470, 1053, 0, 244, 1
    item_properties 27, 0, 0, 5, 0, 0, 0
    item_modifiers 3, 10, 0, 0, 0, 0, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 348, 471, 1054, 0, 244, 1
    item_properties 27, 0, 0, 5, 0, 0, 0
    item_modifiers 0, 3, 0, 0, 0, 0, 2, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 349, 472, 1055, 2000, 232, 3
    item_properties 28, 0, 0, 6, 120, 0, 0
    item_modifiers 5, 5, 0, 0, 0, 0, 0, 0
    item_tail 27, 0, 0, 0, 0, 0, 0, 0

    item_header 350, 473, 1056, 0, 220, 5
    item_properties 28, 0, 0, 6, 0, 0, 0
    item_modifiers 3, 3, 0, 0, 0, 0, 0, 0
    item_tail 27, 0, 60, 0, 0, 0, 0, 0

    item_header 351, 474, 1057, 15000, 244, 1
    item_properties 28, 0, 0, 6, 0, 0, 0
    item_modifiers 5, 12, 0, 0, 0, 0, 0, 0
    item_tail 27, 0, 0, 0, 0, 0, 0, 0

    item_header 352, 475, 1058, 0, 244, 1
    item_properties 28, 0, 0, 6, 0, 0, 0
    item_modifiers 5, 10, 2, 5, 0, 0, 0, 0
    item_tail 27, 0, 0, 0, 0, 0, 0, 0

    item_header 353, 476, 1059, 0, 244, 1
    item_properties 28, 0, 0, 6, 0, 0, 0
    item_modifiers 0, 8, 0, 10, 0, 0, 0, 0
    item_tail 0, 0, 2, 0, 0, 0, 0, 0

    item_header 354, 477, 1060, 0, 244, 1
    item_properties 28, 0, 0, 6, 0, 0, 0
    item_modifiers 3, 6, 0, 8, 0, 5, 0, 0
    item_tail 0, 0, 0, 0, 0, 0, 0, 0

    item_header 355, 478, 1061, 10000, 136, 19
    item_properties 29, 0, 0, 7, 64, 0, 0
    item_modifiers 0, 3, 0, 5, 0, 0, 0, 0
    item_tail 28, 0, 51, 50, 0, 0, 0, 0

    item_header 356, 479, 1062, 0, 176, 4
    item_properties 29, 0, 0, 7, 0, 0, 0
    item_modifiers 0, 0, 3, 10, 0, 0, 0, 0
    item_tail 28, 0, 0, 0, 0, 0, 0, 0

    item_header 357, 480, 1063, 0, 164, 6
    item_properties 29, 0, 0, 7, 0, 0, 0
    item_modifiers 0, 0, 0, 0, 0, 0, 0, 0
    item_tail 28, 0, 58, 64, 0, 0, 0, 0

    item_header 358, 481, 1064, 10000, 136, 19
    item_properties 29, 0, 0, 7, 64, 0, 0
    item_modifiers 0, 2, 0, 8, 0, 0, 0, 0
    item_tail 28, 0, 46, 45, 41, 0, 0, 0

    item_header 359, 482, 1065, 0, 184, 11
    item_properties 29, 0, 0, 7, 0, 0, 0
    item_modifiers 0, 3, 0, 6, 0, 0, 0, 0
    item_tail 28, 0, 49, 44, 38, 0, 0, 0

    item_header 360, 483, 1066, 15000, 244, 1
    item_properties 29, 0, 0, 7, 0, 0, 0
    item_modifiers 0, 4, 6, 4, 2, 0, 0, 0
    item_tail 28, 0, 42, 43, 0, 0, 0, 0

    item_header 361, 484, 1067, 0, 244, 1
    item_properties 29, 0, 0, 7, 0, 0, 0
    item_modifiers 0, 3, 0, 3, 0, 0, 0, 0
    item_tail 28, 0, 59, 28, 0, 0, 0, 0

    item_header 362, 485, 1068, 50, 25, 0
    item_properties 30, 0, 0, 0, 248, 0, 0
    item_modifiers 0, 0, 0, 0, 0, 0, 0, 0
    item_tail 29, 0, 0, 0, 0, 0, 0, 0

    item_header 363, 486, 1069, 200, 100, 0
    item_properties 30, 0, 0, 0, 248, 0, 0
    item_modifiers 0, 0, 0, 0, 0, 0, 0, 0
    item_tail 29, 0, 0, 0, 0, 0, 0, 0

    item_header 364, 487, 1070, 700, 94, 1
    item_properties 30, 0, 0, 0, 248, 0, 0
    item_modifiers 0, 0, 0, 0, 0, 0, 0, 0
    item_tail 29, 0, 0, 0, 0, 0, 0, 0

    item_header 365, 488, 1071, 0, 244, 1
    item_properties 30, 0, 0, 0, 128, 0, 0
    item_modifiers 0, 0, 0, 0, 0, 0, 0, 0
    item_tail 29, 0, 0, 0, 0, 0, 0, 0

    item_header 366, 489, 1072, 20000, 244, 1
    item_properties 30, 0, 0, 0, 128, 0, 0
    item_modifiers 0, 0, 0, 0, 0, 0, 0, 0
    item_tail 29, 0, 0, 0, 0, 0, 0, 0

    item_header 367, 490, 1073, 50, 25, 0
    item_properties 30, 0, 0, 0, 248, 0, 0
    item_modifiers 0, 0, 0, 0, 0, 0, 0, 0
    item_tail 29, 0, 0, 0, 0, 0, 0, 0

    item_header 368, 491, 1074, 50, 25, 0
    item_properties 30, 0, 0, 0, 248, 0, 0
    item_modifiers 0, 0, 0, 0, 0, 0, 0, 0
    item_tail 29, 0, 0, 0, 0, 0, 0, 0

    item_header 369, 492, 1075, 50, 25, 0
    item_properties 30, 0, 0, 0, 248, 0, 0
    item_modifiers 0, 0, 0, 0, 0, 0, 0, 0
    item_tail 29, 0, 0, 0, 0, 0, 0, 0

    item_header 370, 493, 1076, 50, 25, 0
    item_properties 30, 0, 0, 0, 248, 0, 0
    item_modifiers 0, 0, 0, 0, 0, 0, 0, 0
    item_tail 29, 0, 0, 0, 0, 0, 0, 0

    item_header 371, 494, 1077, 100, 50, 0
    item_properties 30, 0, 0, 0, 248, 0, 0
    item_modifiers 0, 0, 0, 0, 0, 0, 0, 0
    item_tail 29, 0, 0, 0, 0, 0, 0, 0

    item_header 372, 495, 1078, 2000, 232, 3
    item_properties 30, 0, 0, 0, 248, 0, 0
    item_modifiers 0, 0, 0, 0, 0, 0, 0, 0
    item_tail 29, 0, 0, 0, 0, 0, 0, 0

    item_header 373, 496, 1079, 200, 100, 0
    item_properties 30, 0, 0, 0, 248, 0, 0
    item_modifiers 0, 0, 0, 0, 0, 0, 0, 0
    item_tail 29, 0, 0, 0, 0, 0, 0, 0

    item_header 374, 497, 1080, 350, 175, 0
    item_properties 30, 0, 0, 0, 128, 0, 0
    item_modifiers 0, 0, 0, 0, 0, 0, 0, 0
    item_tail 29, 0, 0, 0, 0, 0, 0, 0

    item_header 375, 498, 1081, 300, 150, 0
    item_properties 30, 0, 0, 0, 248, 0, 0
    item_modifiers 0, 0, 0, 0, 0, 0, 0, 0
    item_tail 29, 0, 0, 0, 0, 0, 0, 0

.if (. - gItems) != 376 * 32
    .error "item table must contain 376 records"
.endif
.size gItems, . - gItems
