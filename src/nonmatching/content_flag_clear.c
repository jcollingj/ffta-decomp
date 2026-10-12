#include "types.h"

extern u8 GetGameFlag(u16 flag);
extern void SetGameFlag(u16 flag, u16 value);

void ClearContentFlag21FIfSet(void)
{
    if (GetGameFlag(0x21F) == 0)
        return;
    SetGameFlag(0x21F, 0);
}
