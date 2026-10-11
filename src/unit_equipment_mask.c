#include "equipment_scan.h"
#include "item.h"
extern u32 GetUnitProperty(const u8 *unit, u8 property);
extern const u32 gEquipmentClassMasks[];
u8 UnitItemCategoryAllowed(const u8 *unit, u16 item)
{
    u32 mask = gEquipmentClassMasks[GetUnitProperty(unit, 36)];
    u8 category = GetItemProperty(item, 3);
    return (mask & (1u << (category - 1))) != 0;
}
u8 InspectEquipmentMode0(struct EquipmentView *unit, u16 item, u8 slot)
{
    return InspectEquipment(unit, item, slot, 0);
}
