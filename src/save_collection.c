#include "save_collection.h"
/* Address calculation preserves the original base/index/field ordering. */
u8 HasSaveCollectionRecord(void)
{
    u8 i = 0;
    for (; i < 20; i++) {
        const u8 *base = (const u8 *)&gSaveData + i * 16;
        if (*(const u32 *)(base + 0x2e78)) return 1;
    }
    return 0;
}
u8 CountSaveCollectionRecords(void)
{
    u8 i = 0;
    u8 count = 0;
    for (; i < 20; i++) {
        const u8 *base = (const u8 *)&gSaveData + i * 16;
        if (*(const u32 *)(base + 0x2e78)) count++;
    }
    return count;
}
void IncrementSaveCounterBelow99(void)
{
    if (gSaveData.counter < 99) gSaveData.counter++;
}
