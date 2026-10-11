#ifndef FFTA_UNIT_POOL_H
#define FFTA_UNIT_POOL_H
#include "unit.h"
/* Verified record stride and pool counts; gameplay roles remain unknown. */
struct UnitRecord { struct UnitFieldView fields; u8 unknownFC[12]; };
typedef char UnitRecordSize[(sizeof(struct UnitRecord) == 0x108) ? 1 : -1];
extern struct UnitRecord gUnitPoolA[12];
extern struct UnitRecord gUnitPoolB[24];
struct UnitRecord *FindEmptyUnitPoolASlot(void);
#endif
