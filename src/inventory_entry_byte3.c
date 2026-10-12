#include "inventory_internal.h"
void AdjustInventoryEntryByte3(u16 item, u8 increase)
{
    struct InventoryEntry *entry;
    int count, i;
    if (!item) return;
    entry = InventoryForItem(item, &count);
    for (i = 0; i < count; i++, entry++) {
        if (entry->item == item) break;
    }
    if (i == count) return;
    if (increase) {
        if (entry->unknown >= entry->quantity) return;
        entry->unknown++;
    } else {
        if (!entry->unknown) return;
        entry->unknown--;
    }
}
asm(".align 2, 0");
