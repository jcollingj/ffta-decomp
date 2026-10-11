/* NONMATCHING candidate for 0xC8A24..0xC8C24; stack/register layout
 * still differs. Not linked or counted as recovered code. */
#include "job.h"
#include "unit.h"
#include "ability_state.h"
extern u8 GetGameFlag(u16);
extern void SetGameFlag(u16, u16);
extern void (*const gClearMemory)(void *, u32);
struct JobRequirement { u8 job; u8 count; } __attribute__((packed));
extern const struct JobRequirement gJobRequirements[][2];

u8 BuildAvailableJobList(const struct UnitFieldView *unit, u8 *output)
{
    u8 required, satisfied;
    int was_available;
    int job;
    int written = 0;
    int flag;
    gClearMemory(output, 12);
    switch (ReadUnitField(unit, 1)) {
    case 12: SetGameFlag(38, 1); break;
    case 9: case 10: case 94: return 0;
    }
    for (job = 43, flag = 46; job > 1; job--, flag--) {
        u8 category = GetJobProperty(job, job, 1);
        was_available = 0;
        if (category == unit->bytes04[2]) {
            const struct JobRequirement *requirements;
            if (GetGameFlag(flag)) {
                *output = job;
                was_available = 1;
            }
            requirements = gJobRequirements[GetJobProperty(job, job, 39)];
            required = 0;
            satisfied = 0;
            {
                int k;
                for (k = 0; k < 2; k++)
                    if (requirements[k].job) required++;
            }
            if (!required) {
                SetGameFlag(flag, 1);
                *output = job | (signed char)0x80;
                goto append;
            }
            {
            int i;
            u8 prerequisite;
            for (i = 0; i < 2 && (prerequisite = requirements->job) != 0; i++) {
                const struct JobRequirement *current = requirements;
                u8 first = GetJobProperty(prerequisite, prerequisite, 37);
                u8 last = GetJobProperty(prerequisite, prerequisite, 38);
                int mastered = 0;
                int ability;
                for (ability = first; ability <= last; ability++) {
                    if (GetAbilityListEntry(unit->bytes04[2], ability)->category == 1) {
                        const u8 *states = (const u8 *)unit + 0x40;
                        u8 progress = states[ability] & 0x7f;
                        if (progress >= GetAbilityListEntry(unit->bytes04[2], ability)->requirement)
                            mastered++;
                    }
                }
                if (mastered >= current->count) satisfied++;
                else break;
                requirements = current + 1;
            }
            }
            if (required && required == satisfied) {
                SetGameFlag(flag, 1);
                *output++ = job | (signed char)0x80;
                written++;
            } else if (was_available) {
                *output = job;
append:
                output++;
                written++;
            }
        }
    }
    return written;
}
asm(".align 2, 0");
