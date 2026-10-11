#include <assert.h>
#include <string.h>
#include "../../src/unit_fields.c"
#include "../../src/game_flags.c"
#include "../../src/unit_pool_action.c"
#include "../../src/unit_pool_set.c"
#include "../../src/unit_pool_predicate.c"
#include "../../src/unit_pool_process.c"
struct SaveUnitPoolView gSaveData;
u8 gGameFlags[16];
static void expected_action(struct UnitRecord *unit, u8 *flags)
{
    unsigned id = unit->fields.bytes04[0];
    unsigned flag = id == 8 ? 56 : id == 94 ? 70 : id == 4 ? 71 :
                    id == 12 ? 72 : id == 9 ? 73 : 0;
    unit->fields.unknownE8[8] = 0;
    unit->fields.half18[8] &= 0xffe7;
    if (flag) flags[flag / 8] &= ~(1u << (flag % 8));
}
int main(void)
{
    struct UnitRecord unit, expected;
    struct SaveUnitPoolView expected_save;
    u8 expected_flags[16];
    unsigned id, value, i;
    for (value = 0; value < 65536; value++) {
        memset(&unit, 0xa5, sizeof(unit));
        unit.fields.bytes04[0] = value & 255;
        unit.fields.half18[8] = value;
        unit.fields.unknownE8[8] = value >> 8;
        memset(gGameFlags, 0xff, sizeof(gGameFlags));
        expected = unit;
        memcpy(expected_flags, gGameFlags, sizeof(gGameFlags));
        expected_action(&expected, expected_flags);
        ProcessSelectedPoolUnit(&unit);
        assert(memcmp(&unit, &expected, sizeof(unit)) == 0);
        assert(memcmp(gGameFlags, expected_flags, sizeof(gGameFlags)) == 0);
    }
    for (id = 0; id < 256; id++) {
        memset(&unit, 0, sizeof(unit));
        unit.fields.bytes04[0] = id;
        memset(gGameFlags, 0, sizeof(gGameFlags));
        ProcessSelectedPoolUnit(&unit);
        for (i = 0; i < sizeof(gGameFlags); i++) assert(gGameFlags[i] == 0);
    }
    for (id = 0; id < 256; id++) {
        for (value = 0; value < 256; value++) {
            static const unsigned ids[] = {8, 94, 4, 12, 9};
            static const unsigned flags[] = {56, 70, 71, 72, 73};
            memset(&unit, 0xa5, sizeof(unit));
            unit.fields.bytes04[0] = id;
            unit.fields.half18[8] = (id * 257) ^ value;
            memset(gGameFlags, value, sizeof(gGameFlags));
            expected = unit;
            memcpy(expected_flags, gGameFlags, sizeof(gGameFlags));
            expected.fields.unknownE8[8] = value;
            expected.unknownFC[6] = expected.unknownFC[7] = 0;
            expected.fields.half18[8] |= 0x18;
            for (i = 0; i < 5; i++) if (id == ids[i])
                expected_flags[flags[i] / 8] |= 1u << (flags[i] % 8);
            SetSelectedPoolUnitState(&unit, value);
            assert(memcmp(&unit, &expected, sizeof(unit)) == 0);
            assert(memcmp(gGameFlags, expected_flags, sizeof(gGameFlags)) == 0);
            expected_action(&expected, expected_flags);
            ProcessSelectedPoolUnit(&unit);
            assert(memcmp(&unit, &expected, sizeof(unit)) == 0);
            assert(memcmp(gGameFlags, expected_flags, sizeof(gGameFlags)) == 0);
        }
    }
    memset(&gSaveData, 0, sizeof(gSaveData));
    memset(gGameFlags, 0xff, sizeof(gGameFlags));
    for (i = 0; i < 24; i++) {
        gSaveData.units[i].fields.bytes04[0] = i + 1;
        gSaveData.units[i].fields.half18[8] = 0xffff;
        if (i % 3 == 0) gSaveData.units[i].fields.unknownE8[8] = 1;
    }
    expected_save = gSaveData;
    memcpy(expected_flags, gGameFlags, sizeof(gGameFlags));
    for (i = 0; i < 24; i++)
        if (i % 3) expected_action(&expected_save.units[i], expected_flags);
    assert(HasSelectedPoolUnit() == 1);
    ProcessSelectedPoolUnits();
    assert(memcmp(&gSaveData, &expected_save, sizeof(gSaveData)) == 0);
    assert(memcmp(gGameFlags, expected_flags, sizeof(gGameFlags)) == 0);
    assert(HasSelectedPoolUnit() == 0);
    ProcessSelectedPoolUnits();
    assert(memcmp(&gSaveData, &expected_save, sizeof(gSaveData)) == 0);
    return 0;
}
