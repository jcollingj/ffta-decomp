/* Research: tests item-derived availability, including the paired indices. */
#include "unit.h"
#include "item.h"
struct ItemAbilitySet { u8 count; u8 unknown; u8 entries[9][2]; };
struct UnitEquipment { u8 prefix[0x2a]; u16 items[5]; };
extern const struct ItemAbilitySet gItemAbilitySets[];
u8 HasEquipmentAbility(const u8 *unit, u8 ability)
{
    u8 other = 0;
    int passes = 1;
    if (ReadUnitField((const struct UnitFieldView *)unit, 3) == 4) {
        if (ability == 26) { passes = 2; other = 33; }
        else if (ability == 33) { passes = 2; other = 26; }
    }
    while (--passes != -1) {
        int i, j;
        for (i = 0; i < 5; i++) {
            if (((const struct UnitEquipment *)unit)->items[i]) {
                const struct ItemAbilitySet *set = &gItemAbilitySets[GetItemProperty(((const struct UnitEquipment *)unit)->items[i], 18)];
                for (j = 0; j < set->count; j++) {
                    const u8 *pair = set->entries[j];
                    if (pair[0] && pair[0] == unit[7] && pair[1] == ability)
                        return 1;
                }
            }
        }
        ability = other;
    }
    return 0;
}
