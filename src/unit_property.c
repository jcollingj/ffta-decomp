#include "job.h"

/* The complete unit layout remains under investigation. The original reader
 * uses byte offsets 5/7 for its job-property inputs and a name override at 0x100.
 */

u32 GetUnitProperty(const u8 *unit, u8 property)
{
    if (property == 0 && unit[0x100] != 0)
        return unit[0x100];
    return GetJobProperty(unit[5], unit[7], property);
}

u16 GetUnitNameStringId(const u8 *unit)
{
    u32 name;
    if (unit[0x100] != 0)
        name = unit[0x100];
    else
        name = GetJobProperty(unit[5], unit[7], 0);
    return name + 0x247;
}
