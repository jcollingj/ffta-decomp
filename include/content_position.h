#ifndef FFTA_CONTENT_POSITION_H
#define FFTA_CONTENT_POSITION_H

#include "content_consumer.h"

/* The original routines only establish byte truncation and signed position
 * arithmetic. These names describe the observed coordinate interface. */
u32 GetContentCoordinateQueryValue(struct UnitFieldView *object,
                                   signed int x, signed int y);
u32 ReadContentUnitCoordinateValue(struct ContentConsumerEntry *entry,
                                   signed int x, signed int y);
void SetContentEntryPosition(struct ContentConsumerEntry *entry,
                             signed int x, signed int y);

#endif
