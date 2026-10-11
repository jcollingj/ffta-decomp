@ Verified Thumb entry points in still-copied regions; names are provisional.
.thumb
.global ReadCurrentContentIndex
.type ReadCurrentContentIndex, %function
.thumb_set ReadCurrentContentIndex, 0x0800A019
.global ApplyUnitJob
.type ApplyUnitJob, %function
.thumb_set ApplyUnitJob, 0x080C8C25
.global ReadJobRelatedValue
.type ReadJobRelatedValue, %function
.thumb_set ReadJobRelatedValue, 0x080C9135
.global ReadClassValue
.type ReadClassValue, %function
.thumb_set ReadClassValue, 0x080CCAE9
.global _call_via_r2
.type _call_via_r2, %function
.thumb_set _call_via_r2, 0x0814224D
.global _call_via_r3
.type _call_via_r3, %function
.thumb_set _call_via_r3, 0x08142251


.global ReplaceEquipment
.type ReplaceEquipment, %function
.thumb_set ReplaceEquipment, 0x080CAF79



.global __divsi3
.type __divsi3, %function
.thumb_set __divsi3, 0x081426B9



.global __udivsi3
.type __udivsi3, %function
.thumb_set __udivsi3, 0x08142AB1


.global __modsi3
.type __modsi3, %function
.thumb_set __modsi3, 0x08142951

.global EquipmentProvidesAbility
.type EquipmentProvidesAbility, %function
.thumb_set EquipmentProvidesAbility, 0x080CD1FD
.global CheckEquipmentSet
.type CheckEquipmentSet, %function
.thumb_set CheckEquipmentSet, 0x080CABA9
