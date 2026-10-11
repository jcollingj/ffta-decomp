#include "unit.h"
extern u8 ReadJobMappingValue(u8);
u8 UnitMatchesJobMappingSet(const struct UnitFieldView *unit)
{
    switch (ReadJobMappingValue(unit->bytes04[3])) {
    case 1: case 4: case 6: case 7: case 10: case 12:
    case 16: case 21: case 24: case 30: return 1;
    default: return 0;
    }
}
asm(".align 2, 0");
