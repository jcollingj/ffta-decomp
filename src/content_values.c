#include "types.h"
/* Record extent and field roles remain provisional. */
extern u8 GetGameFlag(u16);
extern u16 ReadCurrentContentIndex(void);
struct ContentRecord { u8 unknown00[28]; const u8 *values; u8 unknown20[8]; };
extern const struct ContentRecord gContentRecords[];
extern u8 gContentState[];

u8 ReadContentValue0(void)
{
    if (GetGameFlag(0x253)) {
        return gContentState[0xa6];
    } else {
        u16 index = ReadCurrentContentIndex();
        const struct ContentRecord *record = &gContentRecords[index];
        return record->values[0];
    }
}
u8 ReadContentValue1(void)
{
    if (GetGameFlag(0x253)) {
        return gContentState[0xa7];
    } else {
        u16 index = ReadCurrentContentIndex();
        const struct ContentRecord *record = &gContentRecords[index];
        return record->values[1];
    }
}
