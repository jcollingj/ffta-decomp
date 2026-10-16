#ifndef FFTA_CONTENT_CONSUMER_H
#define FFTA_CONTENT_CONSUMER_H

#include "types.h"

/* The disassembly establishes these pointer layers, not their field meanings. */
struct ContentConsumerFlagObject {
    u8 unknown00[40];
    u16 flags;
};

struct ContentConsumerEntry {
    struct ContentConsumerFlagObject *object;
};

struct ContentConsumerContext {
    u8 unknown00[16];
    u16 flags;
};

/* Return object from the alternate selection helper; only halfword +0x5c is
 * observed by this consumer. */
struct ContentConsumerSelection {
    u8 unknown00[0x5c];
    u16 value5c;
};

/* Generic name: the routine's relationship to maps or missions is unknown. */
void ConsumeContentRecord(struct ContentConsumerContext *input,
                          const void *selector,
                          u16 *first_output,
                          u16 *second_output);

#endif
