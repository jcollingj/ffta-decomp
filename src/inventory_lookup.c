#include "inventory.h"
#include "inventory_internal.h"
u8 GetInventoryQuantityDifference(u16 item)
{
    struct InventoryEntry *entry;
    int count, i;
    if (!item) return 0;
    entry = InventoryForItem(item, &count);
    for (i = 0; i < count; i++, entry++) {
        if (entry->item == item) return entry->quantity - entry->unknown;
    }
    return 0;
}
u8 HasInventoryItem(u16 item)
{
    struct InventoryEntry *entry;
    int count, i;
    if (!item) return 0;
    entry = InventoryForItem(item, &count);
    for (i = 0; i < count; i++, entry++) {
        if (entry->item == item && entry->quantity) return 1;
    }
    return 0;
}
