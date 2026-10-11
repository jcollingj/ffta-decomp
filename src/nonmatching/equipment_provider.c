#include "unit.h"
#include "item.h"
#include "item_abilities.h"
u8 EquipmentProvidesAbility(const u8 *unit, u8 ability)
{
    u8 paired = 0;
    int passes = 1;
    if (ReadUnitField((const struct UnitFieldView *)unit, 3) == 4) {
        if (ability == 26) { passes = 2; paired = 33; }
        else if (ability == 33) { passes = 2; paired = 26; }
    }
    while (--passes != -1) {
        int i;
        const u16 *equipment = (const u16 *)(unit + 0x2a);
        for (i = 0; i < 5; i++) {
            u16 item = equipment[i];
            if (item) {
                const struct ItemAbilitySet *set = &gItemAbilitySets[GetItemProperty(item, 18)];
                int j;
                for (j = 0; j < set->count; j++) {
                    const u8 *entry = set->entries[j];
                    if (entry[0] && entry[0] == unit[7] &&
                        entry[1] == ability) return 1;
                }
            }
        }
        ability = paired;
    }
    return 0;
}
