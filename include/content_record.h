#ifndef FFTA_CONTENT_RECORD_H
#define FFTA_CONTENT_RECORD_H

#include "types.h"

struct ContentSourceEntry {
    u8 bytes[48];
};

/* Observed target fields: u16 count at +0, source pointer at +4, and values
 * pointer at +0x1c. The 20 opaque bytes retain their observed extent on every
 * ABI; native host pointer alignment expands the record naturally. */
struct ContentRecord {
    u16 sourceCount;
    u16 unknown02;
    const struct ContentSourceEntry *sourceEntries;
    u8 unknown08[20];
    const u8 *values;
    u8 unknown20[8];
};

typedef char ContentRecordTargetSize[
    sizeof(void *) != 4 || sizeof(struct ContentRecord) == 40 ? 1 : -1];

extern const struct ContentRecord gContentRecords[];

#endif
