#include "item.h"
int CompareUnsignedBytes(const void *first, const void *second)
{
    return *(const u8 *)first - *(const u8 *)second;
}
int CompareUnsignedHalfwords(const void *first, const void *second)
{
    return *(const u16 *)first - *(const u16 *)second;
}
u8 ItemProperty07Bit1(u16 item)
{
    return (GetItemProperty(item, 7) >> 1) & 1;
}
asm(".align 2, 0");
