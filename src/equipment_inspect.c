#include "equipment_scan.h"
#include "item.h"
#include "item_abilities.h"
#include "ability_state.h"
extern void (*const gCopyRecordMemory)(void *, const void *, u32);
extern u8 EquipmentProvidesAbility(const u8 *, u8);
extern u8 CheckEquipmentSet(struct EquipmentView *, const u16 *, signed char, u8);
u32 InspectEquipment(struct EquipmentView *unit, u16 item, u8 slot, u8 mode)
{
    u16 items[5];
    u8 result = 0;
    u16 equipped;
    gCopyRecordMemory(items, unit->items, 10);
    equipped = items[slot];
    if (!equipped) items[slot] = item;
    else {
        const struct ItemAbilitySet *set = &gItemAbilitySets[GetItemProperty(equipped, 18)];
        u8 *selected = (u8 *)unit + 0x34;
        u16 previous = unit->items[slot];
        unit->items[slot] = 0;
        if (previous != item) {
            int i;
            for (i = 0; i < set->count; i++) {
                if (set->entries[i][0] && set->entries[i][0] == ((u8 *)unit)[7]) {
                    u8 ability = set->entries[i][1];
                    if ((((struct UnitAbilityStateView *)unit)->states[ability] & 0x7f) < GetAbilityListEntry(((u8 *)unit)[6], ability)->requirement &&
                        !EquipmentProvidesAbility((u8 *)unit, ability)) {
                        if (selected[8] == ability) result |= 8;
                        else if (selected[7] == ability) result |= 4;
                        else if (selected[6] == ability) result |= 2;
                    }
                }
            }
        }
        unit->items[slot] = previous;
        items[slot] = item;
    }
    result |= CheckEquipmentSet(unit, items, mode ? (signed char)slot : -1, result);
    return result;
}
asm(".align 2, 0");
