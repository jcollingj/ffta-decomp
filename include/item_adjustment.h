#ifndef FFTA_ITEM_ADJUSTMENT_H
#define FFTA_ITEM_ADJUSTMENT_H
#include "types.h"
/* Recovered prefix only; complete record extent is not implied. */
struct ItemAdjustmentView { u32 category_mask; u16 items[30]; } __attribute__((packed));
const struct ItemAdjustmentView *GetItemAdjustmentRecord(u8 selector);
u8 ItemAmountAdjustmentApplies(u8 selector, u16 item);
u8 ItemInAdjustmentList(u8 selector, u16 item);
u16 ReadItemAdjustmentEntry(u8 selector, u8 index);
#endif
