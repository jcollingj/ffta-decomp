#ifndef FFTA_UNIT_STAT_WRAPPERS_H
#define FFTA_UNIT_STAT_WRAPPERS_H

#include "unit_equipped_stats.h"

signed int SelectUnitEquippedStat20Or24(
    const struct UnitFieldView *unit, u8 mask, u8 flag);
signed int SelectUnitEquippedStat20Or24Capped(
    const struct UnitFieldView *unit, u8 mask);
signed int SelectUnitEquippedStat22Or26(
    const struct UnitFieldView *unit, u8 mask, u8 flag);
signed int SelectUnitEquippedStat22Or26Capped(
    const struct UnitFieldView *unit, u8 mask);

signed int GetUnitStat20Capped(const struct UnitFieldView *unit);
signed int GetUnitStat24Capped(const struct UnitFieldView *unit);
signed int GetUnitStat22Capped(const struct UnitFieldView *unit);
signed int GetUnitStat26Capped(const struct UnitFieldView *unit);

#endif
