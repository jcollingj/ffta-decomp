#include "item_adjustment.h"
static inline u16 ReadEntry(u8 selector, u8 index)
{
    const struct ItemAdjustmentView *record = GetItemAdjustmentRecord(selector);
    if (!record || !index) return 0;
    return record->items[index - 1];
}
u8 ItemInAdjustmentList(u8 selector, u16 item)
{
    u8 index;
    GetItemAdjustmentRecord(selector);
    for (index = 0; index <= 30; index++) {
        if (ReadEntry(selector, index) == item) return 1;
    }
    return 0;
}
u16 ReadItemAdjustmentEntry(u8 selector, u8 index)
{
    return ReadEntry(selector, index);
}
