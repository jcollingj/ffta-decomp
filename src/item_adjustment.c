#include "item.h"
#include "item_adjustment.h"
u8 ItemAmountAdjustmentApplies(u8 selector, u16 item)
{
    const struct ItemAdjustmentView *record = GetItemAdjustmentRecord(selector);
    if (record) {
        u32 category = GetItemProperty(item, 3);
        if (record->category_mask & (1u << (category - 1))) return 1;
    }
    return 0;
}
