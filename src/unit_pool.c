#include "unit_pool.h"
struct UnitRecord *FindEmptyUnitPoolASlot(void)
{
    int i;
    struct UnitRecord *unit = gUnitPoolA;
    for (i = 0; i < 12; i++, unit++) {
        if (unit->fields.bytes04[0] == 0) return unit;
    }
    return 0;
}
asm(".align 2, 0");
