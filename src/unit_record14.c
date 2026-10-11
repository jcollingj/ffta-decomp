#include "unit_record14.h"
static inline const struct UnitRecord14 *RecordByIndex(u8 index) { return &gUnitRecords14[index]; }
extern u32 GetUnitProperty(const u8 *, u8);
u8 LoadUnitRecordSlot(const struct UnitFieldView *unit, u8 selector)
{
    if (UnitField01AboveOne(unit)) {
        const struct UnitRecord14 *record = &gUnitRecords14[unit->bytes04[0]];
        if (selector == 1) {
            if (record->source_a)
                gCopyRecordMemory(gRecordCopyBuffer + record->slot_a * 32,
                    ReadRecordCopySource(record->source_a - 1), 32);
            return record->slot_a;
        } else {
            if (record->source_b)
                gCopyRecordMemory(gRecordCopyBuffer + record->slot_b * 32,
                    ReadRecordCopySource(record->source_b - 1), 32);
            return record->slot_b;
        }
    }
    return GetUnitProperty((const u8 *)unit, selector == 1 ? 6 : 7);
}
u16 ReadUnitRecordProperty04(const struct UnitFieldView *unit)
{
    if (UnitField01AboveOne(unit)) return RecordByIndex(unit->bytes04[0])->property04;
    return GetUnitProperty((const u8 *)unit, 4);
}
u16 ReadUnitRecordProperty05(const struct UnitFieldView *unit)
{
    if (UnitField01AboveOne(unit)) return RecordByIndex(unit->bytes04[0])->property05;
    return GetUnitProperty((const u8 *)unit, 5);
}

u8 ReadUnitRecordProperty09(const struct UnitFieldView *unit)
{
    if (UnitField01AboveOne(unit)) return RecordByIndex(unit->bytes04[0])->properties09[0];
    return GetUnitProperty((const u8 *)unit, 9);
}
u8 ReadUnitRecordProperty10Or11(const struct UnitFieldView *unit, u8 selector)
{
    if (UnitField01AboveOne(unit)) {
        const struct UnitRecord14 *record = RecordByIndex(unit->bytes04[0]);
        if (selector == 1) return record->properties09[1];
        return record->properties09[2];
    }
    return GetUnitProperty((const u8 *)unit, selector == 1 ? 10 : 11);
}
