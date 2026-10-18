#include "unit_derived_bytes.h"

int RefreshAndReadUnitDerivedByte(struct UnitRecord *unit)
{
    u8 *derived;
    UpdateUnitDerivedBytes(unit);
    derived = unit->unknownFC;
    return (signed char)derived[2];
}
