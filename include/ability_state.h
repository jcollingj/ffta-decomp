#ifndef FFTA_ABILITY_STATE_H
#define FFTA_ABILITY_STATE_H
#include "types.h"
struct AbilityListEntry { u8 unknown00[4]; u8 value04; u8 unknown05; u8 category; u8 requirement; };
extern const struct AbilityListEntry *const gAbilityLists[];
typedef char AbilityListEntrySize[(sizeof(struct AbilityListEntry) == 8) ? 1 : -1];
const struct AbilityListEntry *GetAbilityListEntry(u8, u8);
u8 IsAbilityAvailable(const u8 *, u8);
/* Partial unit view: extent of the indexed state array is still unknown. */
struct UnitAbilityStateView { u8 prefix[0x40]; u8 states[0]; };
static inline void MarkAbilityAvailable(u8 *unit, u8 ability)
{
    if (unit) ((struct UnitAbilityStateView *)unit)->states[ability] |= 0x80;
}
void SetAbilityAvailable(u8 *, u8);
#endif
