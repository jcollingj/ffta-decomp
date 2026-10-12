#include "ability_state.h"
#include "equipment_scan.h"
#include "item.h"
#include "item_abilities.h"

extern u8 EquipmentProvidesAbility(const u8 *unit, u8 ability);
extern void SyncPairedAbilityState(u8 *unit, u8 ability);
extern void ApplyItemAbilityFlags(u8 *unit, u16 item);

void ReplaceEquipmentDirect(struct EquipmentView *unit, u16 item, u8 slot)
{
    u8 *bytes = (u8 *)unit;
    u16 previous = unit->items[slot];
    u8 *selected = bytes + 0x34;

    if (previous && previous == item) return;
    if (previous) {
        const struct ItemAbilitySet *set = &gItemAbilitySets[GetItemProperty(previous, 18)];
        unsigned i;
        unit->items[slot] = 0;
        for (i = 0; i < set->count; i++) {
            u8 job = set->entries[i][0];
            if (job && job == bytes[7]) {
                u8 ability = set->entries[i][1];
                u8 progress = bytes[0x40 + ability] & 0x7f;
                if (progress < GetAbilityListEntry(bytes[6], ability)->requirement &&
                    !EquipmentProvidesAbility(bytes, ability)) {
                    if (selected[8] == ability) selected[8] = 0;
                    else if (selected[7] == ability) selected[7] = 0;
                    else if (selected[6] == ability) selected[6] = 0;
                    bytes[0x40 + ability] &= 0x7f;
                    SyncPairedAbilityState(bytes, ability);
                }
            } else if (job == 255 && GetItemProperty(previous, 3) == 27) {
                selected[9] = 0;
            }
        }
    }
    unit->items[slot] = item;
    ApplyItemAbilityFlags(bytes, item);
}
