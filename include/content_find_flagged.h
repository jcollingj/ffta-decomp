#ifndef FFTA_CONTENT_FIND_FLAGGED_H
#define FFTA_CONTENT_FIND_FLAGGED_H

#include "content_consumer.h"
#include "list_count_wrappers.h"

/* Return the first entry in context->unknown10 whose unit field 27 has bit
 * 12 set, or NULL when no entry qualifies. */
struct ContentConsumerEntry *FindContentEntryWithUnitFlag12(
    struct ListCountContext *context);

#endif
