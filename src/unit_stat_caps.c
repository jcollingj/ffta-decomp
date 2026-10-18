#include "unit_stat_wrappers.h"

signed int GetUnitStat20Capped(const struct UnitFieldView *unit)
{
    signed int result = GetUnitEquippedStat20(unit, 1);
    if (result > 999)
        result = 999;
    return result;
}

signed int GetUnitStat24Capped(const struct UnitFieldView *unit)
{
    signed int result = GetUnitEquippedStat24(unit, 1);
    if (result > 999)
        result = 999;
    return result;
}

signed int GetUnitStat22Capped(const struct UnitFieldView *unit)
{
    signed int result = GetUnitEquippedStat22(unit, 1);
    if (result > 999)
        result = 999;
    return result;
}

signed int GetUnitStat26Capped(const struct UnitFieldView *unit)
{
    signed int result = GetUnitEquippedStat26(unit, 1);
    if (result > 999)
        result = 999;
    return result;
}
