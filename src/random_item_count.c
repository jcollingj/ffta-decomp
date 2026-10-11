#include "types.h"
extern u16 gRandomItemSlots[5];
u8 CountRandomItemSlots(void)
{
    u8 i = 0;
    u8 count = 0;
    for (; i < 5; i++) if (gRandomItemSlots[i]) count++;
    return count;
}
