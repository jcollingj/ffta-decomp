#include "unit_pool_checks.h"
u8 HasSelectedPoolUnit(void)
{
    u8 i = 0;
    for (; i < 24; i++) {
        const u8 *base = (const u8 *)&gSaveData + i * 0x108;
        if (base[0x84] && !base[0x170] && (*(const u16 *)(base + 0xa8) & 8)) return 1;
    }
    return 0;
}
asm(".align 2, 0");
