#include "item.h"
#include "random.h"
extern const struct Item gItems[];
extern u16 gRandomItemSlots[5];
void FillRandomItemSlots(void)
{
    u8 i = 0;
    const struct Item *items = gItems;
    u16 *slots = gRandomItemSlots;
    for (; i < 5; i++) {
        u16 *slot = &slots[i];
        u16 item;
        do { item = (signed short)NextRandom15() % 376; }
        while (!items[item].unknown18[0]);
        *slot = item;
    }
}
