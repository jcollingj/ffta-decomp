/* Original job/ability progression; names describe observed behavior. */
#include "unit.h"
#include "job.h"
#include "ability_state.h"
extern void SetGameFlag(u16, u16);
static inline const struct AbilityListEntry *JobList(u8 job, u8 list, u8 *first, u8 *last)
{
    u8 start = GetJobProperty(job, job, 37);
    u8 end = GetJobProperty(job, job, 38);
    if (first) *first = start;
    if (last) *last = end;
    return gAbilityLists[list];
}
static inline const struct AbilityListEntry *Entry(u8 list, u8 index)
{
    return &gAbilityLists[list][index];
}
u8 HasMasteredJobAbilities(const u8 *unit, u8 job)
{
    u8 first, last, i;
    const struct AbilityListEntry *list = JobList(job, unit[6], &first, &last);
    for (i = first; i <= last; i++) {
        const struct AbilityListEntry *entry = &list[i];
        if (entry->category) {
            if ((((const struct UnitAbilityStateView *)unit)->states[i] & 0x7f) < Entry(unit[6], i)->requirement)
                return 0;
        }
    }
    return 1;
}
void EnableJobFlagForAbility(const struct UnitFieldView *unit, u8 ability)
{
    int first, last, job;
    if (UnitField27HasBit15(unit)) return;
    if (UnitField27HasBit13(unit)) return;
    first = 2;
    last = 115;
    switch (ReadUnitField(unit, 3)) {
    case 1: first = 2; last = 12; break;
    case 2: first = 13; last = 19; break;
    case 3: first = 20; last = 27; break;
    case 4: first = 28; last = 35; break;
    case 5: first = 36; last = 43; break;
    }
    for (job = first; job <= last; job++) {
        u8 start = GetJobProperty(job, job, 37);
        u8 end = GetJobProperty(job, job, 38);
        if (start <= ability && ability <= end) {
            SetGameFlag(job + 3, 1);
            return;
        }
    }
}
const struct AbilityListEntry *GetJobAbilityList(u8 job, u8 list, u8 *first, u8 *last)
{
    return JobList(job, list, first, last);
}
