#include "types.h"
extern u8 gCollectionBytes[20];
extern u16 gCollectionHalfwords[20];
u8 CountCollectionBytes(void)
{
    u8 i = 0;
    u8 count = 0;
    for (; i < 20; i++) if (gCollectionBytes[i]) count++;
    return count;
}
u8 CountCollectionHalfwords(void)
{
    u8 i = 0;
    u8 count = 0;
    for (; i < 20; i++) if (gCollectionHalfwords[i]) count++;
    return count;
}
void AddCollectionHalfword(u16 value)
{
    u8 i;
    for (i = 0; i < 20; i++) {
        if (!gCollectionHalfwords[i]) {
            gCollectionHalfwords[i] = value;
            return;
        }
    }
}
