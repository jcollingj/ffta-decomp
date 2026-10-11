#ifndef FFTA_EQUIPMENT_SCAN_H
#define FFTA_EQUIPMENT_SCAN_H
#include "types.h"
/* Partial unit view and offset-named result mask; full semantics unresolved. */
struct EquipmentView { u8 prefix[0x2a]; u16 items[5]; };
extern u32 InspectEquipment(struct EquipmentView *, u16, u8, u8);
extern void ReplaceEquipment(struct EquipmentView *, u16, u8);
u8 ScanEquipmentMask81(struct EquipmentView *unit, u8 clear);
#endif
