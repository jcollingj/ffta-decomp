#ifndef FFTA_CONTENT_COORDINATE_SELECT_H
#define FFTA_CONTENT_COORDINATE_SELECT_H

#include "content_consumer.h"
#include "list_visit.h"

/* Native query layout for list callbacks that compare entry coordinates.
 * Only the byte selectors and selected-entry output are currently known. */
struct ContentCoordinateQuery {
    u32 unknown00;
    signed char x;
    signed char y;
    u8 unknown06[2];
    struct ContentConsumerEntry *selected;
};

u32 ContentCoordinateSelectCallback(struct ListCountNode *node, void *context);

#endif
