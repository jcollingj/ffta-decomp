#ifndef FFTA_UNIT_EQUIPMENT_SUMS_H
#define FFTA_UNIT_EQUIPMENT_SUMS_H
#include "unit.h"
#include "types.h"

int ReadUnitEquipmentValue16(const struct UnitFieldView *unit);
int ReadUnitEquipmentValue16Scaled10(const struct UnitFieldView *unit);
u32 SumUnitEquipmentProperty(const struct UnitFieldView *unit, u32 selector);

#endif
