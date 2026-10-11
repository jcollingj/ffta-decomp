/* ROM 0x0CCE60. Names for the external property readers are provisional.
 * Unit offsets remain explicit until the complete unit layout is recovered.
 * first_out/second_out receive property selectors 37/38; null outputs are allowed.
 */
#include "job.h"
extern u32 GetUnitProperty(const u8 *, u8);
extern const void *const gAbilityLists[];
const void *SelectUnitAbilityList(const u8 *unit, u8 slot, u8 *first_out, u8 *second_out)
{
    u32 selected = 0;
    u32 index = 0;
    u8 second = 0;
    u8 first = 0;
    switch (slot) {
    case 1:
        first = GetUnitProperty(unit, 37);
        second = GetUnitProperty(unit, 38);
        selected = unit[0x35];
        index = unit[6];
        break;
    case 2:
        first = GetJobProperty(unit[8], unit[8], 37);
        second = GetJobProperty(unit[8], unit[8], 38);
        selected = unit[0x36];
        index = unit[6];
        break;
    case 3:
        first = GetJobProperty(1, 1, 37);
        second = GetJobProperty(1, 1, 38);
        selected = unit[0x37];
        break;
    }
    if (slot != 3 && selected == 1) {
        first = GetJobProperty(1, 1, 37);
        second = GetJobProperty(1, 1, 38);
        index = 0;
    }
    if (first_out) *first_out = first;
    if (second_out) *second_out = second;
    return gAbilityLists[index];
}
