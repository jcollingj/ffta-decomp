#ifndef FFTA_CONTENT_CONSUMER_H
#define FFTA_CONTENT_CONSUMER_H

#include "unit.h"

/* Target entry: object pointer at +0, value5c at +0x5C. The intervening
 * bytes are opaque; native host offsets expand with pointer width. */
struct ContentConsumerEntry {
    struct UnitFieldView *object;
    u8 unknown04[0x58];
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
