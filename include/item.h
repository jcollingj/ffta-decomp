#ifndef FFTA_ITEM_H
#define FFTA_ITEM_H
#include "types.h"
struct Item {
    u16 name;
    u16 property01;
    u16 property02;
    u16 half06;
    u8 properties03[6];
    u16 property09;
    signed char modifiers[8];
    u8 unknown18[5];
    u8 ability_set;
    u8 unknown1e[2];
};
typedef char ItemSize[(sizeof(struct Item) == 32) ? 1 : -1];
u8 JobAllowsItem(u8 job, u16 item);
u32 CalculateItemAmount(u8 selector, u16 item_id);
u32 GetItemProperty(u16, u8);
#endif
