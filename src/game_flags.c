#include "types.h"

/* Bit storage at EWRAM 0x02001F70. Allocation extent and individual flag names
 * remain under investigation. Original routines intentionally do no bounds check.
 */
extern u8 gGameFlags[];

u8 GetGameFlag(u16 flag)
{
    u8 *byte = &gGameFlags[flag / 8];
    u8 mask = 1 << (flag & 7);
    if (*byte & mask)
        return 1;
    else
        return 0;
}

void SetGameFlag(u16 flag, u16 value)
{
    u8 *byte = &gGameFlags[flag / 8];
    u8 mask = 1 << (flag & 7);
    *byte &= ~mask;
    if (value)
        *byte |= mask;
}
