#include "unit_stat_wrappers.h"

signed int SelectUnitEquippedStat20Or24(
    const struct UnitFieldView *unit, u8 mask, u8 flag)
{
    if (mask & 1)
        return GetUnitEquippedStat20(unit, flag);
    if (mask & 2)
        return GetUnitEquippedStat24(unit, flag);
    return 0;
}

signed int SelectUnitEquippedStat20Or24Capped(
    const struct UnitFieldView *unit, u8 mask)
{
    signed int result;
    if (mask & 1)
        result = GetUnitEquippedStat20(unit, 1);
    else if (mask & 2)
        result = GetUnitEquippedStat24(unit, 1);
    else
        result = 0;
    if (result > 999)
        result = 999;
    return result;
}

signed int SelectUnitEquippedStat22Or26(
    const struct UnitFieldView *unit, u8 mask, u8 flag)
{
    if (mask & 1)
        return GetUnitEquippedStat22(unit, flag);
    if (mask & 2)
        return GetUnitEquippedStat26(unit, flag);
    return 0;
}

signed int SelectUnitEquippedStat22Or26Capped(
    const struct UnitFieldView *unit, u8 mask)
{
    signed int result;
    if (mask & 1)
        result = GetUnitEquippedStat22(unit, 1);
    else if (mask & 2)
        result = GetUnitEquippedStat26(unit, 1);
    else
        result = 0;
    if (result > 999)
        result = 999;
    return result;
}
