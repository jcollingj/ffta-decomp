#include "unit_reference_encode.h"

u32 EncodeUnitReferencePointer(const struct PackedUnitReferencePointer *reference)
{
    u32 i;
    for (i = 0; i <= 724; i++) {
        if (gUnitReferenceTable0[i] == reference->value)
            return i;
    }
    for (i = 0; i <= 106; i++) {
        if (gUnitReferenceTable1[i] == reference->value)
            return i | 0x8000;
    }
    return 0;
}
