/* Research: original equipment-derived ability list builders. Not yet counted. */
#include "unit.h"
#include "ability_state.h"
#include "item.h"
struct ItemAbilitySet { u8 count; u8 unknown; u8 entries[9][2]; };
struct UnitAbilityListView { u8 prefix[7]; u8 job; u8 unknown08[34]; u16 equipment[5]; u8 count; };
extern const struct ItemAbilitySet gItemAbilitySets[];
extern void (*const gClearMemory)(void *, u32);
static inline u8 Available(u8 *unit, u8 ability)
{
    if (unit) {
        const u8 *states = unit + 0x40;
        if (states[ability] & 0x80) return 1;
    }
    return 0;
}
u8 BuildAvailableAbilityList(u8 *unit, u8 *output, u8 category)
{
    u8 equipment[32];
    u8 *cursor = output;
    const struct AbilityListEntry *const *lists = gAbilityLists;
    const struct AbilityListEntry *entry = lists[ReadUnitField((const struct UnitFieldView *)unit, 3)] + 1;
    int count, i, j;
    gClearMemory(equipment, 32);
    count = 0;
    for (i = 0; i < 5; i++) {
        if (((const struct UnitAbilityListView *)unit)->equipment[i]) {
            const struct ItemAbilitySet *set = &gItemAbilitySets[GetItemProperty(((const struct UnitAbilityListView *)unit)->equipment[i], 18)];
            for (j = 0; j < set->count; j++) {
                if (set->entries[j][0] && set->entries[j][0] == unit[7])
                    equipment[count++] = set->entries[j][1];
            }
        }
    }
    for (i = 1; i < unit[0x34]; i++, entry++) {
        if (entry->category == category) {
            if (Available(unit, i)) *cursor++ = i;
            else for (j = 0; j < count; j++) {
                if (equipment[j] == i) *cursor++ = i;
            }
        }
    }
    return cursor - output;
}
u8 BuildProgressAbilityList(u8 *unit, u8 *output, u8 category)
{
    u8 equipment[32];
    u8 *cursor = output;
    const struct AbilityListEntry *const *lists = gAbilityLists;
    const struct AbilityListEntry *entry = lists[ReadUnitField((const struct UnitFieldView *)unit, 3)] + 1;
    int count, i, j;
    gClearMemory(equipment, 32);
    count = 0;
    for (i = 0; i < 5; i++) {
        if (((const struct UnitAbilityListView *)unit)->equipment[i]) {
            const struct ItemAbilitySet *set = &gItemAbilitySets[GetItemProperty(((const struct UnitAbilityListView *)unit)->equipment[i], 18)];
            for (j = 0; j < set->count; j++) {
                if (set->entries[j][0] && set->entries[j][0] == unit[7])
                    equipment[count++] = set->entries[j][1];
            }
        }
    }
    for (i = 1; i < unit[0x34]; i++, entry++) {
        if (entry->category == category) {
            if (unit[0x40 + i]) *cursor++ = i;
            else for (j = 0; j < count; j++) {
                if (equipment[j] == i) *cursor++ = i;
            }
        }
    }
    return cursor - output;
}
asm(".align 2, 0");
