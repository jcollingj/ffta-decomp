#include "unit_pool_checks.h"
extern void SetGameFlag(u16, u16);
void ProcessSelectedPoolUnit(struct UnitRecord *unit)
{
    unit->fields.unknownE8[8] = 0;
    unit->fields.half18[8] &= ~8;
    unit->fields.half18[8] &= ~16;
    if (UnitField01Equals(&unit->fields, 8)) SetGameFlag(56, 0);
    if (UnitField01Equals(&unit->fields, 94)) SetGameFlag(70, 0);
    if (UnitField01Equals(&unit->fields, 4)) SetGameFlag(71, 0);
    if (UnitField01Equals(&unit->fields, 12)) SetGameFlag(72, 0);
    if (UnitField01Equals(&unit->fields, 9)) SetGameFlag(73, 0);
}
