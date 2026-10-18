#include "unit_equipment_sums.h"
#include "item.h"

u32 SumUnitEquipmentProperty(const struct UnitFieldView *unit, u32 selector)
{
    u8 property = (u8)selector;
    u32 sum = 0;
    const u16 *equipment = &unit->half18[9];
    int i;

    for (i = 0; i <= 4; i++) {
        u16 item = equipment[i];
        if (item != 0)
            sum += GetItemProperty(item, property);
    }

    return sum;
}
