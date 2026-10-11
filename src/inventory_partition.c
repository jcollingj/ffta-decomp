#include "inventory.h"
extern struct InventoryEntry gInventoryEntries[];
u8 IsItemInRange1To252(u16 item)
{
    return (u16)(item - 1) <= 251;
}
struct InventoryEntry *GetInventoryPartition(u16 item, int *count)
{
    struct InventoryEntry *entries = gInventoryEntries;
    u32 size = 361;
    if (item > size && item <= 375) {
        entries = &gInventoryEntries[361];
        size = 14;
    }
    if (count) *count = size;
    return entries;
}
