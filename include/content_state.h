#ifndef FFTA_CONTENT_STATE_H
#define FFTA_CONTENT_STATE_H

#include "types.h"

/* Only bytes +0x00 through +0xA7 are characterized by current readers. */
struct ContentIndexState {
    u8 unknown00[0xa0];
    u16 currentIndex;  /* +0xa0 */
    u16 fallbackIndex; /* +0xa2 */
    u8 unknownA4[2];   /* +0xa4..+0xa5 */
    u8 value0;         /* +0xa6 */
    u8 value1;         /* +0xa7 */
};

extern struct ContentIndexState gContentState;

#endif
