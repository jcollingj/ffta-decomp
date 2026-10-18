#ifndef FFTA_UNIT_EQUIPPED_STATS_H
#define FFTA_UNIT_EQUIPPED_STATS_H
#include "unit.h"
signed int GetUnitEquippedStat20(const struct UnitFieldView *unit, u8 flag);
signed int GetUnitEquippedStat24(const struct UnitFieldView *unit, u8 flag);
signed int GetUnitEquippedStat22(const struct UnitFieldView *unit, u8 flag);
signed int GetUnitEquippedStat26(const struct UnitFieldView *unit, u8 flag);
#endif
