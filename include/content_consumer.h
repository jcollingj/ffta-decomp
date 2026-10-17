#ifndef FFTA_CONTENT_CONSUMER_H
#define FFTA_CONTENT_CONSUMER_H

#include "unit.h"

/* Target entry: object pointer at +0; signed positions at +8 and +12;
 * value5c at +0x5C. The remaining fields are opaque. Native host offsets
 * expand with pointer width. */
struct ContentConsumerEntry {
    struct UnitFieldView *object;
    u32 unknown04;
    signed short position08;
    u8 unknown0A[2];
    signed short position0C;
    u8 unknown0E[0x4E];
    u16 value5c;
};

struct ContentConsumerContext {
    u8 unknown00[16];
    u16 flags;
};

/* Generic name: the routine's relationship to maps or missions is unknown. */
void ConsumeContentRecord(struct ContentConsumerContext *input,
                          const void *selector,
                          u16 *first_output,
                          u16 *second_output);

#endif
