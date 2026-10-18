#include "unit_content_setup.h"

#include "inventory.h"
#include "item_abilities.h"
#include "unit_record14.h"

extern u32 GetUnitProperty(const u8 *unit, u8 property);
extern int AddInventoryItem(u16 item, u8 quantity);

void ApplyUnitContentProperty33(u8 *unit)
{
    u32 property = GetUnitProperty(unit, 33);

    if (property != 0) {
        u8 *destination = unit + 0x40;
        destination += property;
        *destination = 0xE4;
    }

    if (unit[7] != 0) {
        if ((u8)(unit[7] - 68) <= 1)
            ((struct UnitFieldView *)unit)->half18[8] |= 0x0800;
    }
}

void LoadUnitContentEquipment(u8 *unit,
                             const struct ContentSourceEntry *source)
{
    u8 addItems = 1;
    u16 *items;
    int i;

    gCopyRecordMemory(unit + 0x2A, source->bytes + 8, 10);

    if (UnitField27HasBit15((const struct UnitFieldView *)unit))
        addItems = 0;
    else if (UnitField27HasBit13((const struct UnitFieldView *)unit))
        addItems = 0;

    if (ReadUnitField((const struct UnitFieldView *)unit, 3) == 23 &&
        !UnitField01Equals((const struct UnitFieldView *)unit, 94))
        addItems = 0;

    items = (u16 *)(unit + 0x2A);
    for (i = 4; i >= 0; i--) {
        if (addItems != 0) {
            AddInventoryItem(items[0], 1);
            AdjustInventoryEntryByte3(items[0], 1);
        }
        ApplyItemAbilityFlags(unit, items[0]);
        items++;
    }
}
