/* Unit offsets and ability indices remain provisional until callers are recovered. */
#include "unit.h"

/* Returns 312..316 from selector 3; the consumer at 0x09403A stores this
 * as a halfword. The exact UI/content meaning is not yet established. */
u32 SelectUnitAbilityValue(const u8 *unit)
{
    switch (ReadUnitField((const struct UnitFieldView *)unit, 3)) {
    case 1:
    case 23: return 313;
    case 2: return 312;
    case 3:
    case 20: return 314;
    case 4: return 315;
    default: return 316;
    }
}

void SyncPairedAbilityState(u8 *unit, u8 ability)
{
    u32 other;
    u8 *states;
    if (ReadUnitField((const struct UnitFieldView *)unit, 3) == 4) {
        if (ability == 26) other = 33;
        else if (ability == 33) other = 26;
        else return;
        states = unit + 0x40;
        states[other] = states[ability];
    }
}

static inline u8 ReadSlot(const u8 *unit, u8 slot)
{
    if (unit) return unit[slot];
    return 0;
}

void ClearMatchingAbilitySlot(u8 *unit, u8 ability)
{
    if (ability == ReadSlot(unit, 0x3a)) unit[0x3a] = 0;
    else if (ability == ReadSlot(unit, 0x3b)) unit[0x3b] = 0;
    else if (ability == ReadSlot(unit, 0x3c)) unit[0x3c] = 0;
}
