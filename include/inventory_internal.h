#ifndef FFTA_INVENTORY_INTERNAL_H
#define FFTA_INVENTORY_INTERNAL_H
#include "inventory.h"
/* Original inline partition helper; keep inlining at its existing callers. */
extern struct InventoryEntry gInventoryEntries[];
static inline struct InventoryEntry *InventoryForItem(u16 item, int *count)
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
#endif
