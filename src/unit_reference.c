#include "unit_reference.h"

const u8 *ResolveUnitReference(const struct UnitRecord *unit)
{
    u32 index = unit->fields.word00;
    switch (unit->unknownFC[10]) {
    case 0: return gUnitReferenceTable0[index];
    case 1: return gUnitReferenceTable1[index];
    default: return gUnitReferenceFallback;
    }
}

const u8 *ResolveEncodedUnitReference(const struct EncodedUnitReference *encoded)
{
    u32 index = encoded->value;
    if (index > 0x7FFF)
        return gUnitReferenceTable1[index & 0x7FFF];
    else
        return gUnitReferenceTable0[index];
}

asm(".align 2,0");
