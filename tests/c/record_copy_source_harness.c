#include <assert.h>
#include "../../src/record_copy_source.c"
/* Large host fixtures permit every byte index; not claims about ROM bank size. */
const u8 gRecordSources0[256 * 32] = {0};
const u8 gRecordSources1[256 * 32] = {0};
const u8 gRecordSources2[256 * 32] = {0};
struct RecordCopySaveView gSaveData;
int main(void)
{
    unsigned mode, index;
    for (mode = 0; mode < 256; mode++) {
        const u8 *base = (mode & 3) == 1 ? gRecordSources1 :
                        (mode & 3) == 2 ? gRecordSources2 : gRecordSources0;
        ((u8 *)&gSaveData)[0x2fc2] = mode;
        for (index = 0; index < 256; index++) {
            assert(ReadRecordCopySource(index) == base + index * 32);
            assert(((u8 *)&gSaveData)[0x2fc2] == mode);
        }
    }
    return 0;
}
