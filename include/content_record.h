#ifndef FFTA_CONTENT_RECORD_H
#define FFTA_CONTENT_RECORD_H

#include "types.h"

/* The native pointer at +0x1c and eight trailing bytes make a 40-byte target
 * record. Host builds retain native pointer width, so their stride is larger. */
struct ContentRecord {
    u8 unknown00[28];
    const u8 *values;
    u8 unknown20[8];
};

typedef char ContentRecordTargetSize[
    sizeof(void *) != 4 || sizeof(struct ContentRecord) == 40 ? 1 : -1];

extern const struct ContentRecord gContentRecords[];

#endif
