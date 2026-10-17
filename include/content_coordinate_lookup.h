#ifndef FFTA_CONTENT_COORDINATE_LOOKUP_H
#define FFTA_CONTENT_COORDINATE_LOOKUP_H

#include "content_coordinate_select.h"
#include "list_count_wrappers.h"

struct ContentConsumerEntry *FindContentEntryByPosition(
    struct ListCountContext *context, int x, int y);
struct ContentConsumerEntry *FindContentEntryByUnitCoordinates(
    struct ListCountContext *context, int x, int y);

#endif
