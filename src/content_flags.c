#include "types.h"

/* Numeric save-field names are provisional; the surrounding save schema is
 * still unknown. This view describes only the observed offsets.
 */
struct ContentFlagSaveView {
    u8 prefix[0x2192];
    u16 halfword2192;
    u16 halfword2194;
};

extern struct ContentFlagSaveView gSaveData;
extern u8 GetGameFlag(u16 flag);
extern void SetGameFlag(u16 flag, u16 value);

u8 GetContentFlag21F(void)
{
    return GetGameFlag(0x21F);
}

void CopySaveHalfword2194To2192AndSetFlag21F(void)
{
    gSaveData.halfword2192 = gSaveData.halfword2194;
    SetGameFlag(0x21F, 1);
}
