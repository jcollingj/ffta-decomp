#ifndef FFTA_UNIT_RECORD14_H
#define FFTA_UNIT_RECORD14_H
#include "unit.h"
/* Partial format; table extent and field meanings remain unverified. */
struct UnitRecord14 {
    u8 unknown00[4];
    u16 property04, property05;
    u32 slot_a:4, source_a:8, slot_b:4, source_b:8;
    u8 properties09[3];
} __attribute__((packed));
extern const struct UnitRecord14 gUnitRecords14[];
extern u8 gRecordCopyBuffer[];
extern void (*const gCopyRecordMemory)(void *, const void *, u32);
extern const void *ReadRecordCopySource(u8);
typedef char UnitRecord14Size[(sizeof(struct UnitRecord14) == 14) ? 1 : -1];
u8 LoadUnitRecordSlot(const struct UnitFieldView *, u8);
u16 ReadUnitRecordProperty04(const struct UnitFieldView *);
u16 ReadUnitRecordProperty05(const struct UnitFieldView *);
u8 ReadUnitRecordProperty09(const struct UnitFieldView *);
u8 ReadUnitRecordProperty10Or11(const struct UnitFieldView *, u8);
#endif
