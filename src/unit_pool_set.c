#include "unit_pool_checks.h"
extern void SetGameFlag(u16, u16);
void SetSelectedPoolUnitState(struct UnitRecord *unit, u8 value)
{
    unit->fields.unknownE8[8] = value;
    unit->unknownFC[6] = 0;
    unit->unknownFC[7] = 0;
    if (UnitField01Equals(&unit->fields, 8)) SetGameFlag(56, 1);
    if (UnitField01Equals(&unit->fields, 94)) SetGameFlag(70, 1);
    if (UnitField01Equals(&unit->fields, 4)) SetGameFlag(71, 1);
    if (UnitField01Equals(&unit->fields, 12)) SetGameFlag(72, 1);
    if (UnitField01Equals(&unit->fields, 9)) SetGameFlag(73, 1);
    unit->fields.half18[8] |= 8;
    unit->fields.half18[8] |= 16;
}
