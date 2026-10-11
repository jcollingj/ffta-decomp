#include "ability_state.h"
const struct AbilityListEntry *GetAbilityListEntry(u8 list, u8 index)
{
    return &gAbilityLists[list][index];
}
static inline u8 ReadSlot(const u8 *unit, u8 offset)
{
    u8 value;
    if (unit) value = unit[offset];
    else value = 0;
    return value;
}
u8 ReadAbilitySlotA(const u8 *unit) { return ReadSlot(unit, 0x3a); }
u8 ReadAbilitySlotB(const u8 *unit) { return ReadSlot(unit, 0x3b); }
u8 ReadAbilitySlotC(const u8 *unit) { return ReadSlot(unit, 0x3c); }
u8 ReadAbilitySlotAValue(const u8 *unit)
{
    if (!ReadSlot(unit, 0x3a)) return 0;
    else {
        u8 list = unit[6];
        u8 index = unit[0x3a];
        const struct AbilityListEntry *entry = &gAbilityLists[list][index];
        return entry->value04;
    }
}
u8 ReadAbilitySlotBValue(const u8 *unit)
{
    if (!ReadSlot(unit, 0x3b)) return 0;
    else {
        u8 list = unit[6];
        u8 index = unit[0x3b];
        const struct AbilityListEntry *entry = &gAbilityLists[list][index];
        return entry->value04;
    }
}
asm(".align 2, 0");
