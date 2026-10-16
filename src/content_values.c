#include "types.h"
#include "content_record.h"
#include "content_state.h"
/* Record extent and field roles remain provisional. */
extern u8 GetGameFlag(u16);
extern u16 ReadCurrentContentIndex(void);

u8 ReadContentValue0(void)
{
    if (GetGameFlag(0x253)) {
        return gContentState.value0;
    } else {
        u16 index = ReadCurrentContentIndex();
        const struct ContentRecord *record = &gContentRecords[index];
        return record->values[0];
    }
}
u8 ReadContentValue1(void)
{
    if (GetGameFlag(0x253)) {
        return gContentState.value1;
    } else {
        u16 index = ReadCurrentContentIndex();
        const struct ContentRecord *record = &gContentRecords[index];
        return record->values[1];
    }
}
