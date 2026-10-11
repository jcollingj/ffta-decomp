#include "types.h"
extern const u8 gClassRecords4[][4], gClassRecords16[][16];
extern u8 gCollectionBytes[20];
extern u16 gCollectionHalfwords[20];
u8 ReadCollectionClass(u16 index)
{
    u16 mapped = index;
    if (index > 170) return gClassRecords4[index - 171][0];
    if (index > 85) mapped = index - 85;
    return gClassRecords16[mapped][0];
}
