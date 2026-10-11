#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "../../src/unit_pool_predicate.c"
#include "../../src/unit_pool_process.c"
struct SaveUnitPoolView gSaveData;
static unsigned calls[24], count, mutate_next;
void ProcessSelectedPoolUnit(struct UnitRecord *unit)
{
    unsigned index = unit - gSaveData.units;
    assert(index < 24 && count < 24);
    calls[count++] = index;
    if (mutate_next && index + 1 < 24) gSaveData.units[index + 1].fields.bytes04[0] = 0;
}
int main(void)
{
    static const u8 bytes[] = {0, 1, 255};
    static const u16 flags[] = {0, 8, 0xffff, 0xfff7};
    struct SaveUnitPoolView expected;
    unsigned slot, present, blocked, flag, i;
    assert(offsetof(struct SaveUnitPoolView, units) == 0x80);
    for (slot = 0; slot < 24; slot++) {
        for (present = 0; present < 3; present++) {
            for (blocked = 0; blocked < 3; blocked++) {
                for (flag = 0; flag < 4; flag++) {
                    struct UnitFieldView *unit = &gSaveData.units[slot].fields;
                    unsigned qualifies = present && !blocked && (flags[flag] & 8);
                    memset(&gSaveData, 0, sizeof(gSaveData));
                    unit->bytes04[0] = bytes[present];
                    unit->unknownE8[8] = bytes[blocked];
                    unit->half18[8] = flags[flag];
                    expected = gSaveData;
                    count = 0;
                    assert(HasSelectedPoolUnit() == (qualifies != 0));
                    ProcessSelectedPoolUnits();
                    assert(count == (qualifies != 0));
                    if (count) assert(calls[0] == slot);
                    assert(memcmp(&gSaveData, &expected, sizeof(expected)) == 0);
                }
            }
        }
    }
    memset(&gSaveData, 0, sizeof(gSaveData));
    for (i = 0; i < 24; i++) {
        gSaveData.units[i].fields.bytes04[0] = 1;
        gSaveData.units[i].fields.half18[8] = 8;
    }
    count = 0;
    ProcessSelectedPoolUnits();
    assert(count == 24);
    for (i = 0; i < 24; i++) assert(calls[i] == i);
    count = 0;
    mutate_next = 1;
    ProcessSelectedPoolUnits();
    assert(count == 12);
    for (i = 0; i < 12; i++) assert(calls[i] == i * 2);
    return 0;
}
