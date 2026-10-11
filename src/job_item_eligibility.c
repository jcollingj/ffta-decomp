#include "job.h"
#include "item.h"
extern const u32 gEquipmentClassMasks[];
u8 JobAllowsItem(u8 job, u16 item)
{
    u8 group = GetJobProperty(job, job, 1);
    u32 mask = gEquipmentClassMasks[GetJobProperty(job, job, 36)];
    u8 category = GetItemProperty(item, 3);
    if (category == 22) return group == 4;
    if (!(mask & (1u << (category - 1)))) return 0;
    if (category == 25 || category == 23) {
        if (item == 311 || item == 316 || item == 287) {
            if (group != 4) return 0;
        }
    }
    return 1;
}
