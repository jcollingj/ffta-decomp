#include "ability_state.h"
#include "item.h"
#include "item_abilities.h"
extern void SyncPairedAbilityState(u8 *, u8);
static inline void SetSpecialAbility(u8 *unit, u8 ability)
{
    unit[0x3d] = ability;
}
void ApplyItemAbilityFlags(u8 *unit, u16 item)
{
    const struct ItemAbilitySet *set;
    int i;
    if (item == 0) return;
    set = &gItemAbilitySets[GetItemProperty(item, 18)];
    for (i = 0; i < set->count; i++) {
        if (unit[7] == set->entries[i][0]) {
            MarkAbilityAvailable(unit, set->entries[i][1]);
            SyncPairedAbilityState(unit, set->entries[i][1]);
        } else if (set->entries[i][0] == 255) {
            SetSpecialAbility(unit, set->entries[i][1]);
        }
    }
}
