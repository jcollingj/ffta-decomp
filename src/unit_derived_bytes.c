#include "unit_derived_bytes.h"
#include "unit_equipment_sums.h"
#include "unit_status.h"

u32 GetUnitProperty(const u8 *unit, u8 property);

void UpdateUnitDerivedBytes(struct UnitRecord *unit)
{
    u8 first = (u8)GetUnitProperty((const u8 *)unit, 34);
    u8 second = (u8)GetUnitProperty((const u8 *)unit, 35);
    u8 third = (u8)GetUnitProperty((const u8 *)unit, 31);

    if (UnitStateE9Bit0((const u8 *)unit) && first != 1)
        first = 1;

    switch (unit->fields.unknown34[9]) {
    case 2:
        third++;
        break;
    case 3:
        third += 2;
        break;
    case 4:
        second = 5;
        break;
    case 5:
        if ((u8)(second - 1) <= 1)
            second = 3;
        break;
    case 6:
        first = 3;
        break;
    }

    third += SumUnitEquipmentProperty(&unit->fields, 17);
    {
        u8 *derived = unit->unknownFC;
        derived[0] = first;
        derived[1] = second;
        derived[2] = third;
        derived[3] = ~third;
    }
}

asm(".align 2,0");
