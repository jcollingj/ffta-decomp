#include "unit_clamped_values.h"
#include "item.h"

extern u32 GetUnitProperty(const u8 *unit, u8 property);

signed int ReadUnitEquipmentValue14(const struct UnitFieldView *unit)
{
    u16 base = (u16)unit->signedD0[1];
    u32 sum = 0;
    signed int i = 0;

    for (i = 0; i != 5; i++) {
        u16 item = unit->half18[9 + i];
        if (item != 0)
            sum += GetItemProperty(item, 14);
    }

    {
        signed int result = (signed short)((signed short)base + sum);
        if (result < 0)
            result = 0;
        return result;
    }
}

signed int ReadUnitEquipmentValue15(const struct UnitFieldView *unit)
{
    u16 base = (u16)GetUnitProperty((const u8 *)unit, 32);
    u32 sum = 0;
    signed int result;
    signed int i = 0;

    for (i = 0; i != 5; i++) {
        u16 item = unit->half18[9 + i];
        if (item != 0)
            sum += GetItemProperty(item, 15);
    }

    result = (signed short)((signed short)base + sum);
    if (result < 0)
        return 0;
    if (result > 100)
        return 100;
    return result;
}
