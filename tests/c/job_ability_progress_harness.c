/* Actual recovered control flow with property/flag dependencies isolated. */
#include <assert.h>
#include <string.h>
#include "../../src/job_ability_progress.c"

static u8 ranges[116][2];
static struct AbilityListEntry entries[4];
const struct AbilityListEntry *const gAbilityLists[] = {entries};
static unsigned property_calls, flag_calls, written_flag;
u32 GetJobProperty(u8 job, u8 fallback, u8 selector)
{
    assert(job < 116 && fallback == job);
    assert(selector == 37 || selector == 38);
    property_calls++;
    return ranges[job][selector - 37];
}
u32 ReadUnitField(const struct UnitFieldView *unit, u8 selector)
{
    assert(selector == 3);
    return unit->bytes04[2];
}
u8 UnitField27HasBit15(const struct UnitFieldView *unit)
{ return (unit->half18[8] & 0x8000) != 0; }
u8 UnitField27HasBit13(const struct UnitFieldView *unit)
{ return (unit->half18[8] & 0x2000) != 0; }
void SetGameFlag(u16 flag, u16 value)
{
    assert(value == 1);
    flag_calls++;
    written_flag = flag;
}
static void test_mastery(void)
{
    u8 unit[0x108], first, last;
    memset(unit, 0, sizeof(unit));
    ranges[2][0] = 1;
    ranges[2][1] = 3;
    assert(GetJobAbilityList(2, 0, &first, &last) == entries);
    assert(first == 1 && last == 3);
    first = 0;
    assert(GetJobAbilityList(2, 0, &first, 0) == entries && first == 1);
    last = 0;
    assert(GetJobAbilityList(2, 0, 0, &last) == entries && last == 3);
    assert(GetJobAbilityList(2, 0, 0, 0) == entries);
    entries[1].category = 1;
    entries[1].requirement = 10;
    entries[2].category = 0;
    entries[2].requirement = 100; /* Category zero is excluded. */
    entries[3].category = 2;      /* Other nonzero categories still count. */
    entries[3].requirement = 20;
    unit[0x41] = 10;
    unit[0x43] = 20;
    assert(HasMasteredJobAbilities(unit, 2) == 1);
    unit[0x43] = 19;
    assert(HasMasteredJobAbilities(unit, 2) == 0);
    unit[0x43] = 0x80 | 19;
    assert(HasMasteredJobAbilities(unit, 2) == 0); /* Availability is not progress. */
    unit[0x43] = 0x80 | 20;
    assert(HasMasteredJobAbilities(unit, 2) == 1);
    unit[0x41] = 9;
    assert(HasMasteredJobAbilities(unit, 2) == 0);
    ranges[2][0] = 3;
    ranges[2][1] = 1;
    assert(HasMasteredJobAbilities(unit, 2) == 1); /* Empty range. */
}
static void test_job_flags(void)
{
    static const u8 starts[] = {2, 2, 13, 20, 28, 36};
    static const u8 ends[] = {115, 12, 19, 27, 35, 43};
    struct UnitFieldView unit;
    unsigned category, endpoint, target;
    memset(&unit, 0, sizeof(unit));
    for (category = 0; category < 6; category++) {
        unit.bytes04[2] = category;
        for (endpoint = 0; endpoint < 2; endpoint++) {
            memset(ranges, 0, sizeof(ranges));
            target = endpoint ? ends[category] : starts[category];
            ranges[target][0] = 10;
            ranges[target][1] = 11;
            flag_calls = 0;
            EnableJobFlagForAbility(&unit, 10);
            assert(flag_calls == 1 && written_flag == target + 3);
            flag_calls = 0;
            EnableJobFlagForAbility(&unit, 11);
            assert(flag_calls == 1 && written_flag == target + 3);
            flag_calls = 0;
            EnableJobFlagForAbility(&unit, 12);
            assert(flag_calls == 0);
        }
        /* Every job matches: only the first in the selected range is flagged. */
        memset(ranges, 10, sizeof(ranges));
        flag_calls = 0;
        EnableJobFlagForAbility(&unit, 10);
        assert(flag_calls == 1 && written_flag == starts[category] + 3u);
    }
    unit.half18[8] = 0x8000;
    flag_calls = property_calls = 0;
    EnableJobFlagForAbility(&unit, 10);
    assert(flag_calls == 0 && property_calls == 0);
    unit.half18[8] = 0x2000;
    EnableJobFlagForAbility(&unit, 10);
    assert(flag_calls == 0 && property_calls == 0);
}
int main(void)
{
    test_mastery();
    test_job_flags();
    return 0;
}
