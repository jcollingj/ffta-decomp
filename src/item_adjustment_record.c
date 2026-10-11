#include "item_adjustment.h"
extern const struct ItemAdjustmentView gItemAdjustment2[], gItemAdjustment3[],
    gItemAdjustment4[], gItemAdjustment5[], gItemAdjustment6[];
const struct ItemAdjustmentView *GetItemAdjustmentRecord(u8 selector)
{
    switch (selector) {
    case 2: return gItemAdjustment2;
    case 3: return gItemAdjustment3;
    case 4: return gItemAdjustment4;
    case 5: return gItemAdjustment5;
    case 6: return gItemAdjustment6;
    default: return 0;
    }
}
asm(".align 2, 0");
