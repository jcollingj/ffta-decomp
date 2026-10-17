@ Verified Thumb entry points in still-copied regions; names are provisional.
.thumb
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
.global _call_via_r6
.type _call_via_r6, %function
.thumb_set _call_via_r6, 0x0814225D
.global _call_via_r7
.type _call_via_r7, %function
.thumb_set _call_via_r7, 0x08142261
.global _call_via_r8
.type _call_via_r8, %function
.thumb_set _call_via_r8, 0x08142265


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

.global AllocatorInternalAllocate
.type AllocatorInternalAllocate, %function
.thumb_set AllocatorInternalAllocate, 0x08006EC1
.global AllocatorInternalRelease
.type AllocatorInternalRelease, %function
.thumb_set AllocatorInternalRelease, 0x08006FCD
.global AllocatorRecenter
.type AllocatorRecenter, %function
.thumb_set AllocatorRecenter, 0x08006D39
.global AllocatorFindNeighbors
.type AllocatorFindNeighbors, %function
.thumb_set AllocatorFindNeighbors, 0x08006C89

.global DetachTreeNode
.type DetachTreeNode, %function
.thumb_set DetachTreeNode, 0x080C7859

@ RAM globals read by the recovered content context getters.
.global gContentConsumerContext
.type gContentConsumerContext, %object
.set gContentConsumerContext, 0x0200F4B0
.global gContentGlobalPointerOwner
.type gContentGlobalPointerOwner, %object
.set gContentGlobalPointerOwner, 0x0200F4A8
