#include "save_value21b4.h"
extern void SetGameFlag(u16, u16);
void SetFlag60AtSelectedValues(void)
{
    u32 value = gSaveData.value;
    if (value == 10 || value == 20 || value == 30 || value == 40 ||
        value == 50 || value == 60 || value == 70 || value == 80 || value == 90)
        SetGameFlag(60, 1);
    else if (value == 96)
        SetGameFlag(60, 1);
}
