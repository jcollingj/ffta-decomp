#ifndef FFTA_ITEM_ABILITIES_H
#define FFTA_ITEM_ABILITIES_H
#include "types.h"
/* Count, unknown byte, and job/ability-index pairs. Job 255 has a special
 * path in ApplyItemAbilityFlags; unused pair slots remain part of the record. */
struct ItemAbilitySet { u8 count; u8 unknown; u8 entries[9][2]; };
typedef char ItemAbilitySetSize[(sizeof(struct ItemAbilitySet) == 20) ? 1 : -1];
extern const struct ItemAbilitySet gItemAbilitySets[];
void ApplyItemAbilityFlags(u8 *, u16);
#endif
