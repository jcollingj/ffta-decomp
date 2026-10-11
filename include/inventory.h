#ifndef FFTA_INVENTORY_H
#define FFTA_INVENTORY_H
#include "types.h"
struct InventoryEntry { u16 item; u8 quantity; u8 unknown; };
typedef char InventoryEntrySize[(sizeof(struct InventoryEntry) == 4) ? 1 : -1];
struct InventoryEntry *GetInventoryPartition(u16 item, int *count);
u8 IsItemInRange1To252(u16 item);
u8 GetInventoryItemClass(u16 item);
u16 CopyInventoryClass(struct InventoryEntry *output, u8 category);
u16 FindInventoryItemIndex(u16 item);
u8 ClassifyValueByTens(u16 value);
#endif
