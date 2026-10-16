#ifndef FFTA_ABILITY_H
#define FFTA_ABILITY_H

#include "types.h"

/* Layout recovered from ROM 0x55187C and GetAbilityProperty at 0x0CCD50.
 * Unknown fields remain explicitly named; no semantic claims from offsets alone.
 */
struct Ability {
    u16 name_id;
    u8 category;
    u8 unknown_03;
    u8 mp_cost;
    u8 unknown_05;
    u8 unknown_06;
    u8 unknown_07;
    u8 range;
    u8 area;
    u8 unknown_0a;
    u8 power;
    u8 effects[4];
    u32 flags;
    u16 unknown_14;
    u8 unknown_16[6];
};

typedef char AbilitySizeMustBe28[(sizeof(struct Ability) == 28) ? 1 : -1];

extern const struct Ability gAbilities[];
u32 GetAbilityProperty(u16 ability_id, u8 property);

#endif
