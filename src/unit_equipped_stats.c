#include "unit_equipped_stats.h"
#include "item.h"

signed int GetUnitEquippedStat20(const struct UnitFieldView *unit, u8 flag)
{
    signed short base = unit->half18[4];
    signed int result;
    if (flag) {
        u32 equipment = 0;
        const u16 *itemPointer = &unit->half18[9];
        signed int i = 4;
        do {
            u16 item = *itemPointer;
            if (item) equipment += GetItemProperty(item, 10);
            itemPointer++;
        } while (--i >= 0);
        base += (signed short)(u16)equipment;
    }
    result = base;
    if (result > 999) result = 999;
    return result;
}

signed int GetUnitEquippedStat24(const struct UnitFieldView *unit, u8 flag)
{
    signed short base = unit->half18[6];
    signed int result;
    if (flag) {
        u32 equipment = 0;
        const u16 *itemPointer = &unit->half18[9];
        signed int i = 4;
        do {
            u16 item = *itemPointer;
            if (item) equipment += GetItemProperty(item, 12);
            itemPointer++;
        } while (--i >= 0);
        base += (signed short)(u16)equipment;
    }
    result = base;
    if (result > 999) result = 999;
    return result;
}

signed int GetUnitEquippedStat22(const struct UnitFieldView *unit, u8 flag)
{
    signed short base = unit->half18[5];
    signed int result;
    if (flag) {
        u32 equipment = 0;
        const u16 *itemPointer = &unit->half18[9];
        signed int i = 4;
        do {
            u16 item = *itemPointer;
            if (item) equipment += GetItemProperty(item, 11);
            itemPointer++;
        } while (--i >= 0);
        base += (signed short)(u16)equipment;
    }
    result = base;
    if (result > 999) result = 999;
    return result;
}

signed int GetUnitEquippedStat26(const struct UnitFieldView *unit, u8 flag)
{
    signed short base = unit->half18[7];
    signed int result;
    if (flag) {
        u32 equipment = 0;
        const u16 *itemPointer = &unit->half18[9];
        signed int i = 4;
        do {
            u16 item = *itemPointer;
            if (item) equipment += GetItemProperty(item, 13);
            itemPointer++;
        } while (--i >= 0);
        base += (signed short)(u16)equipment;
    }
    result = base;
    if (result > 999) result = 999;
    return result;
}
