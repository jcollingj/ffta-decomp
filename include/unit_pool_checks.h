#ifndef FFTA_UNIT_POOL_CHECKS_H
#define FFTA_UNIT_POOL_CHECKS_H
#include "unit_pool.h"
/* Save prefix precedes the verified 24-record pool. */
struct SaveUnitPoolView { u8 prefix[0x80]; struct UnitRecord units[24]; };
extern struct SaveUnitPoolView gSaveData;
extern void ProcessSelectedPoolUnit(struct UnitRecord *);
u8 HasSelectedPoolUnit(void);
void ProcessSelectedPoolUnits(void);
void SetSelectedPoolUnitState(struct UnitRecord *, u8 value);
#endif
