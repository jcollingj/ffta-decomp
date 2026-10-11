#include "inventory.h"
#include "item.h"
u8 GetInventoryItemClass(u16 item)
{
    if (GetItemProperty(item, 6) == 3) return 0;
    if (GetItemProperty(item, 6) == 4) return 1;
    if (IsItemInRange1To252(item)) return 2;
    if (GetItemProperty(item, 3) == 20) return 3;
    if (GetItemProperty(item, 3) == 29 || GetItemProperty(item, 6) == 5 || GetItemProperty(item, 3) == 28) return 4;
    if ((u16)(item - 362) <= 13) return 5;
    return 255;
}
