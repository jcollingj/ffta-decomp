#include "types.h"
struct RecordCopySaveView { u8 prefix[0x2fc2]; u32 mode:2; } __attribute__((packed));
extern struct RecordCopySaveView gSaveData;
extern const u8 gRecordSources0[], gRecordSources1[], gRecordSources2[];
const void *ReadRecordCopySource(u8 index)
{
    switch (gSaveData.mode) {
    case 2: return gRecordSources2 + index * 32;
    case 1: return gRecordSources1 + index * 32;
    default: return gRecordSources0 + index * 32;
    }
}
