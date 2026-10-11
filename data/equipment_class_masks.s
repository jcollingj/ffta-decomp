@ Category mask lookup selected by job property 36.
@ Bit (item property 3 - 1) controls the ordinary eligibility path.
.section .text
.global gEquipmentClassMasks
.type gEquipmentClassMasks, %object
gEquipmentClassMasks:
.macro equipment_class_mask index, mask
    .if (\index != ((. - gEquipmentClassMasks) / 4))
        .error "equipment mask indices must be consecutive"
    .endif
    .if (\mask < 0) || (\mask > 0xffffffff)
        .error "equipment mask must fit an unsigned word"
    .endif
    .long \mask
.endm
    equipment_class_mask 0, 0x00000000
    equipment_class_mask 1, 0x1dd80011
    equipment_class_mask 2, 0x1e980018
    equipment_class_mask 3, 0x1d400002
    equipment_class_mask 4, 0x1d400040
    equipment_class_mask 5, 0x1d400100
    equipment_class_mask 6, 0x1f600200
    equipment_class_mask 7, 0x1f400400
    equipment_class_mask 8, 0x1f400400
    equipment_class_mask 9, 0x1f400004
    equipment_class_mask 10, 0x1d401000
    equipment_class_mask 11, 0x1d402000
    equipment_class_mask 12, 0x1dd80021
    equipment_class_mask 13, 0x1c904001
    equipment_class_mask 14, 0x1e900028
    equipment_class_mask 15, 0x1d400002
    equipment_class_mask 16, 0x1d010000
    equipment_class_mask 17, 0x1f400200
    equipment_class_mask 18, 0x1fd04008
    equipment_class_mask 19, 0x1f400400
    equipment_class_mask 20, 0x1d400800
    equipment_class_mask 21, 0x1d408000
    equipment_class_mask 22, 0x1f420000
    equipment_class_mask 23, 0x1f480800
    equipment_class_mask 24, 0x1d680080
    equipment_class_mask 25, 0x1f600080
    equipment_class_mask 26, 0x1f600080
    equipment_class_mask 27, 0x1f600200
    equipment_class_mask 28, 0x1d602100
    equipment_class_mask 29, 0x1d602000
    equipment_class_mask 30, 0x1d408000
    equipment_class_mask 31, 0x1dd80002
    equipment_class_mask 32, 0x1d440000
    equipment_class_mask 33, 0x1d400040
    equipment_class_mask 34, 0x1d410000
.size gEquipmentClassMasks, . - gEquipmentClassMasks
.if (. - gEquipmentClassMasks) != 140
    .error "equipment masks must contain 35 records"
.endif
