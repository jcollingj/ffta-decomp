/* Research candidate, not linked or counted. Original interval 0xC8364..0xC8570. */
#include "unit.h"
#include "unit_pool.h"
#include "content_record.h"
#include "content_state.h"
extern u8 ReadJobMappingValue(u8);
extern u8 GetGameFlag(u16);
extern u16 ReadCurrentContentIndex(void);

u8 UnitField27HasBit8(const struct UnitFieldView *unit)
{
    u8 result = 0;
    if (unit) {
        u16 bits = unit->half18[8] & 0x100;
        result = bits != 0;
    }
    return result;
}
u8 UnitField03InRange6To18(const struct UnitFieldView *unit)
{
    u8 value;
    u8 result;
    if (!unit) return 0;
    value = unit->bytes04[2];
    result = 0;
    if ((u8)(value - 6) <= 12) result = 1;
    return result;
}
struct UnitRecord *FindEmptyUnitPoolASlot(void)
{
    int i;
    struct UnitRecord *unit = gUnitPoolA;
    for (i = 0; i < 12; i++, unit++) {
        if (unit->fields.bytes04[0] == 0) return unit;
    }
    return 0;
}
static inline struct UnitRecord *GetPoolA(int *count)
{ if (count) *count = 12; return gUnitPoolA; }
static inline struct UnitRecord *GetPoolB(int *count)
{ if (count) *count = 24; return gUnitPoolB; }
struct UnitRecord *FindUnitByField01(u8 value)
{
    struct UnitRecord *unit;
    int count, i;
    if (value <= 1) return 0;
    {
        unit = GetPoolA(&count);
        for (i = 0; i < count; i++, unit++)
            if (unit->fields.bytes04[0] == value) return unit;
        unit = GetPoolB(&count);
        for (i = 0; i < count; i++, unit++)
            if (unit->fields.bytes04[0] == value) return unit;
    }
    return 0;
}
u8 UnitMatchesJobMappingSet(const struct UnitFieldView *unit)
{
    switch (ReadJobMappingValue(unit->bytes04[3])) {
    case 1: case 4: case 6: case 7: case 10: case 12:
    case 16: case 21: case 24: case 30: return 1;
    default: return 0;
    }
}
static inline u8 IsSpecialValue(u8 value)
{
    return value == 82 || value == 84 || value == 83 || value == 30 || value == 31
        || (value >= 57 && value <= 70);
}
u8 UnitField01InSpecialSet(const struct UnitFieldView *unit)
{
    u8 result = 0;
    if (unit) result = IsSpecialValue(unit->bytes04[0]);
    return result;
}
u8 ReadContentValue0(void)
{
    if (GetGameFlag(0x253)) {
        return gContentState.value0;
    } else {
        u16 index = ReadCurrentContentIndex();
        const struct ContentRecord *record = &gContentRecords[index];
        return record->values[0];
    }
}
u8 ReadContentValue1(void)
{
    if (GetGameFlag(0x253)) {
        return gContentState.value1;
    } else {
        u16 index = ReadCurrentContentIndex();
        const struct ContentRecord *record = &gContentRecords[index];
        return record->values[1];
    }
}
