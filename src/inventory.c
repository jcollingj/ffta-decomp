#include "inventory.h"
/* Entry width and partition bounds are recovered from the original searches.
 * Quantity arithmetic intentionally retains byte wrapping before the 99 cap. */
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
int AddInventoryItem(u16 item, u8 quantity)
{
    struct InventoryEntry *entry;
    int count, i;
    u8 found = 0;
    if (!quantity || !item) return -1;
    entry = InventoryForItem(item, &count);
    for (i = 0; i < count; i++, entry++) {
        if (entry->item != 0 && entry->item == item) { found = 1; break; }
    }
    if (!found) {
        entry = InventoryForItem(item, &count);
        for (i = 0; i < count; i++, entry++) {
            if (entry->item == 0) { found = 1; break; }
        }
    }
    if (!found) return -1;
    if (!entry->item) entry->item = item;
    entry->quantity += quantity;
    if (entry->quantity > 99) {
        u8 value = entry->quantity;
        entry->quantity = 99;
        return (signed char)(value - 99);
    } else return 0;
}
int RemoveInventoryItem(u16 item, u8 quantity)
{
    struct InventoryEntry *entry;
    int count, i;
    u8 found = 0;
    if (quantity) {
        if (!item) return -1;
        {
            entry = InventoryForItem(item, &count);
            for (i = 0; i < count; i++, entry++) {
                if (entry->item != 0 && entry->item == item) { found = 1; break; }
            }
        }
        if (!found) return -1;
        if (entry->quantity > quantity) entry->quantity -= quantity;
        else entry->quantity = 0;
    }
    return 0;
}
