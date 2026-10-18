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
    u16 coordinateValue0A;
    signed short position0C;
    u8 unknown0E[0x16];
    u32 unknown24;
    u8 unknown28[0x34];
    u16 value5c;
};

/* Prefix records make target offsets assertable by the old compiler. */
struct ContentConsumerEntryPositionPrefix {
    struct UnitFieldView *object;
    u32 unknown04;
};
struct ContentConsumerEntryKeyPrefix {
    struct UnitFieldView *object;
    u32 unknown04;
    signed short position08;
    u16 coordinateValue0A;
    signed short position0C;
    u8 unknown0E[0x16];
};
struct ContentConsumerEntryValue5cPrefix {
    struct UnitFieldView *object;
    u32 unknown04;
    signed short position08;
    u16 coordinateValue0A;
    signed short position0C;
    u8 unknown0E[0x16];
    u32 unknown24;
    u8 unknown28[0x34];
};

typedef char ContentConsumerEntryTargetKeyOffset[
    sizeof(void *) != 4 ||
    sizeof(struct ContentConsumerEntryKeyPrefix) == 0x24 ? 1 : -1];
typedef char ContentConsumerEntryTargetPositionOffsets[
    sizeof(void *) != 4 ||
    (sizeof(struct ContentConsumerEntryPositionPrefix) == 0x08 &&
     sizeof(struct ContentConsumerEntryPositionPrefix) + sizeof(signed short) == 0x0A &&
     sizeof(struct ContentConsumerEntryPositionPrefix) + sizeof(signed short) +
         sizeof(u16) == 0x0C) ? 1 : -1];
typedef char ContentConsumerEntryTargetExtent[
    sizeof(void *) != 4 || sizeof(struct ContentConsumerEntry) == 0x60 ? 1 : -1];
typedef char ContentConsumerEntryTargetValue5cOffset[
    sizeof(void *) != 4 ||
    sizeof(struct ContentConsumerEntryValue5cPrefix) == 0x5C ? 1 : -1];
typedef char ContentConsumerEntryOpaqueExtents[
    sizeof(((struct ContentConsumerEntry *)0)->unknown0E) == 0x16 &&
    sizeof(((struct ContentConsumerEntry *)0)->unknown28) == 0x34 ? 1 : -1];
typedef char ContentConsumerEntryHostKeyOffset[
    sizeof(void *) != 8 ||
    sizeof(struct ContentConsumerEntryKeyPrefix) == 0x28 ? 1 : -1];
typedef char ContentConsumerEntryHostPositionOffsets[
    sizeof(void *) != 8 ||
    (sizeof(void *) + sizeof(u32) == 0x0C &&
     sizeof(void *) + sizeof(u32) + sizeof(signed short) == 0x0E &&
     sizeof(void *) + sizeof(u32) + sizeof(signed short) + sizeof(u16) == 0x10) ? 1 : -1];
typedef char ContentConsumerEntryHostValue5cOffset[
    sizeof(void *) != 8 ||
    sizeof(struct ContentConsumerEntryValue5cPrefix) == 0x60 ? 1 : -1];
typedef char ContentConsumerEntryHostExtent[
    sizeof(void *) != 8 || sizeof(struct ContentConsumerEntry) == 0x68 ? 1 : -1];

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
