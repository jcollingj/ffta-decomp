#include "job.h"
#include "unit.h"
extern void SetGameFlag(u16, u16);
extern void ApplyUnitJob(u8 *, u8);
extern u32 ReadJobRelatedValue(u8);
extern u8 ReadClassValue(u8);
struct JobMapping { u8 key; u8 value; } __attribute__((packed));
typedef char JobMappingSize[(sizeof(struct JobMapping) == 2) ? 1 : -1];
extern const struct JobMapping gJobMapping[];
extern const struct Job gJobs[];

/* Flag meanings and the operation guarded by flag 62 remain provisional. */
void EnableFixedJobFlags(void)
{
    SetGameFlag(5, 1);
    SetGameFlag(8, 1);
    SetGameFlag(10, 1);
    SetGameFlag(11, 1);
    SetGameFlag(14, 1);
    SetGameFlag(16, 1);
    SetGameFlag(20, 1);
    SetGameFlag(23, 1);
    SetGameFlag(24, 1);
    SetGameFlag(28, 1);
    SetGameFlag(31, 1);
    SetGameFlag(34, 1);
    SetGameFlag(36, 1);
    SetGameFlag(39, 1);
    SetGameFlag(42, 1);
    SetGameFlag(45, 1);
}
void ApplyUnitJobWithFlag(u8 *unit, u8 job)
{
    SetGameFlag(62, 1);
    ApplyUnitJob(unit, job);
    SetGameFlag(62, 0);
}
u8 ReadJobMappingValue(u8 index)
{
    const struct JobMapping *mapping = &gJobMapping[index];
    return mapping->value;
}
u8 FindJobMappingKey(u8 value)
{
    int i;
    const struct JobMapping *mapping = gJobMapping;
    for (i = 0; i < 43; i++, mapping++)
        if (mapping->value == value) return mapping->key;
    return 0;
}
u32 ReadUnitJobRelatedValue(const struct UnitFieldView *unit)
{
    u8 flags = GetJobProperty(unit->bytes04[1], unit->bytes04[3], 40);
    if (!(flags & 1)) return -1;
    else return ReadJobRelatedValue(ReadUnitField(unit, 4));
}
u32 ReadMappedClassValue(u16 value)
{
    u8 index = 0;
    switch (value) {
    case 202: index = 0; break;
    case 203: index = 2; break;
    case 204: index = 5; break;
    case 205: index = 7; break;
    case 206: index = 10; break;
    case 207: index = 12; break;
    case 208: index = 14; break;
    case 209: index = 16; break;
    case 210: index = 18; break;
    }
    return ReadClassValue(index);
}
u8 FindJobForAbilityRange(u8 category, u16 ability)
{
    u16 i;
    if (!ability) return 0;
    for (i = 0; i < 116; i++) {
        const struct Job *record = &gJobs[i];
        if (record->p01 == category) {
            u8 job = i;
            u16 first = GetJobProperty(job, job, 37);
            u16 last = GetJobProperty(job, job, 38);
            if (first <= ability && last >= ability) return job;
        }
    }
    return 0;
}
