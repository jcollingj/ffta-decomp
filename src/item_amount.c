#include "item.h"
#include "item_adjustment.h"
#include "save_value21b4.h"
struct AmountRecord10 { u8 prefix[8]; u8 reduction; u8 unknown09; } __attribute__((packed));
extern const struct Item gItems[];
extern const struct AmountRecord10 gAmountRecords10[];
u32 CalculateItemAmount(u8 selector, u16 item_id)
{
    const struct Item *items = gItems;
    const struct Item *item;
    u32 result;
    item = &items[item_id];
    if (item->properties03[4] & 8) {
        u8 reduction = gAmountRecords10[gSaveData.value].reduction;
        if (ItemAmountAdjustmentApplies(selector, item_id)) reduction += 10;
        result = item->property02 * (100 - reduction) / 100;
        if (result < item->half06) result = item->half06;
    } else result = item->property02;
    if (!result) result = 1;
    return result;
}
asm(".align 2, 0");
