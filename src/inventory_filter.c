#include "inventory.h"
u16 CopyInventoryClass(struct InventoryEntry *output, u8 category)
{
    int count;
    struct InventoryEntry *entries;
    u16 i;
    u16 written;
    if (category != 5) entries = GetInventoryPartition(1, &count);
    else entries = GetInventoryPartition(362, &count);
    i = 0;
    written = 0;
    for (; i < count; i++) {
        if (GetInventoryItemClass(entries[i].item) == category && entries[i].quantity) {
            output[written] = entries[i];
            written++;
        }
    }
    return written;
}
u16 FindInventoryItemIndex(u16 item)
{
    int count;
    struct InventoryEntry *entries = GetInventoryPartition(item, &count);
    u16 i = 0;
    for (; i < count; i++) {
        if (entries[i].item == item && entries[i].quantity) return i;
    }
    return 65535;
}
u8 ClassifyValueByTens(u16 value)
{
    if (value < 10) return 0;
    if (value < 20) return 1;
    return 2;
}
