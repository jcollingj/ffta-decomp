#include "types.h"
u32 CalculateUnitByteAmount(const u8 *unit)
{
    u8 level = unit[9] / 10u;
    u32 level_multiplier = level * 2 + 1;
    u32 scaled_unit_byte = unit[0xf0] * 1000;
    u32 amount = level_multiplier * scaled_unit_byte;
    if (amount > 99999) amount = 99999;
    return amount;
}
