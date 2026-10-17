#ifndef FFTA_CONTENT_POOL_SETUP_H
#define FFTA_CONTENT_POOL_SETUP_H

#include "content_consumer.h"
#include "content_record.h"
#include "list_count_wrappers.h"
#include "unit_pool.h"

/* Populate the global unit pool for a content index, then prepare the
 * occupied entries through the context's content-entry resolver. */
void LoadAndPrepareContentUnitPool(u32 content_index,
                                   struct ListCountContext *context);

/* Typed research alias for the copied dependency at ROM 0x92440. */
struct ContentConsumerEntry *ResolveOrCreateContentUnitEntry(
    struct ListCountContext *context, struct UnitRecord *unit);

#endif
