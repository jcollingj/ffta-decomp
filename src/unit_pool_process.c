#include "unit_pool_checks.h"
void ProcessSelectedPoolUnits(void)
{
    u8 i = 0;
    for (; i < 24; i++) {
        const u8 *base = (const u8 *)&gSaveData + i * 0x108;
        if (base[0x84] && !base[0x170] && (*(const u16 *)(base + 0xa8) & 8))
            ProcessSelectedPoolUnit(&gSaveData.units[i]);
    }
}
