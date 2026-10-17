#ifndef FFTA_CONTENT_SELECTION_LOOKUP_H
#define FFTA_CONTENT_SELECTION_LOOKUP_H

#include "content_consumer.h"
#include "list_count_wrappers.h"
#include "list_visit.h"

/* Target stack context has selector at +0, an unused word at +4, and the
 * selected entry at +8. The fields retain native pointer width on the host. */
struct ContentSelectionLookupContext {
    const void *selector;
    u32 unknown04;
    struct ContentConsumerEntry *selected;
};

u32 ContentSelectionEqualsObject(struct ListCountNode *node, void *context);
struct ContentConsumerEntry *ResolveContentConsumerSelection(
    struct ListCountContext *context, const void *selector);

#endif
