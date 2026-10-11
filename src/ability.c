#include "ability.h"

/* Original selector API; selector 9 returns a ROM pointer, not an effect ID.
 * Callers are responsible for supplying a valid ability ID (no bounds check).
 */
u32 GetAbilityProperty(u16 ability_id, u8 property)
{
    const struct Ability *ability = &gAbilities[ability_id];
    u8 bit = property - 11;
    if (bit <= 20)
        return (ability->flags & (1 << bit)) != 0;

    switch (property) {
    case 0: return ability->name_id;
    case 1: return ability->category;
    case 2: return ability->mp_cost;
    case 3: return ability->unknown_05;
    case 4: return ability->unknown_06;
    case 5: return ability->unknown_07;
    case 6: return ability->range;
    case 7: return ability->area;
    case 8: return ability->unknown_0a;
    case 9: return (u32)ability->effects;
    case 10: return ability->flags;
    case 32: return ability->unknown_14;
    case 33: return ability->power;
    default: return 0;
    }
}
