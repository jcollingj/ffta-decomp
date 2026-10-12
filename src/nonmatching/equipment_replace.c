#include "equipment_scan.h"
#include "unit.h"
#include "item.h"
#include "item_abilities.h"
#include "ability_state.h"

extern u8 EquipmentProvidesAbility(const u8 *, u8);
extern u8 UnitField27HasBit15(const struct UnitFieldView *);
extern u8 UnitField27HasBit13(const struct UnitFieldView *);
extern u8 GetGameFlag(u16);
extern void AdjustInventoryEntryByte3(u16, u8);
extern void SyncPairedAbilityState(u8 *, u8);
extern void ApplyItemAbilityFlags(u8 *, u16);

void ReplaceEquipment(struct EquipmentView *unit, u16 item, u8 slot)
{
    u16 oldItem = unit->items[slot];

    if (oldItem) {
        if (oldItem == item)
            return;
        {
            const struct ItemAbilitySet *set =
                &gItemAbilitySets[GetItemProperty(oldItem, 18)];
            const u8 *entry = set->entries[0];
            const u8 *end = entry + set->count * 2;

            unit->items[slot] = 0;
            while (entry < end) {
                u8 job = entry[0];
                u8 ability = entry[1];

                if (job && job == ((u8 *)unit)[7]) {
                    if ((((struct UnitAbilityStateView *)unit)->states[ability] & 0x7f) <
                        GetAbilityListEntry(((u8 *)unit)[6], ability)->requirement &&
                        !EquipmentProvidesAbility((u8 *)unit, ability)) {
                        if (((u8 *)unit)[0x3c] == ability)
                            ((u8 *)unit)[0x3c] = 0;
                        else if (((u8 *)unit)[0x3b] == ability)
                            ((u8 *)unit)[0x3b] = 0;
                        else if (((u8 *)unit)[0x3a] == ability)
                            ((u8 *)unit)[0x3a] = 0;
                        ((struct UnitAbilityStateView *)unit)->states[ability] &= 0x7f;
                        SyncPairedAbilityState((u8 *)unit, ability);
                    }
                }
                else if (job == 255 && GetItemProperty(oldItem, 3) == 27)
                    ((u8 *)unit)[0x3d] = 0;
                entry += 2;
            }
        }
    }

    if (!UnitField27HasBit15((const struct UnitFieldView *)unit) &&
        !UnitField27HasBit13((const struct UnitFieldView *)unit) &&
        !GetGameFlag(62)) {
        if (oldItem)
            AdjustInventoryEntryByte3(oldItem, 0);
        if (item)
            AdjustInventoryEntryByte3(item, 1);
    }

    unit->items[slot] = item;
    ApplyItemAbilityFlags((u8 *)unit, item);
}
