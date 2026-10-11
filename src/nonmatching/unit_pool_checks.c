#include "types.h"
u32 CalculateUnitByteAmount(const u8 *unit)
{
    u8 level = unit[9] / 10u;
    u32 amount = (level * 2 + 1) * (unit[0xf0] * 1000);
    if (amount > 99999) amount = 99999;
    return amount;
}
