#include "unit_equipment_sums.h"
#include "item.h"

extern u32 GetUnitProperty(const u8 *unit, u8 property);

int ReadUnitEquipmentValue16(const struct UnitFieldView *unit)
{
    signed char base = (signed char)(u8)GetUnitProperty((const u8 *)unit, 30);
    u32 sum;
    const u16 *equipment;
    int i;

    if (unit->unknown34[9] == 1)
        base++;

    sum = 0;
    equipment = &unit->half18[9];
    i = 4;
    do {
        u16 item = *equipment;
        if (item != 0)
            sum += GetItemProperty(item, 16);
        equipment++;
    } while (--i >= 0);

    base += (signed char)(u8)sum;
    if (base < 0)
        base = 0;
    return base;
}

int ReadUnitEquipmentValue16Scaled10(const struct UnitFieldView *unit)
{
    return ReadUnitEquipmentValue16(unit) * 10;
}

asm(".align 2,0");
