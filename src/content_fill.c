#include "content_fill.h"

#include "content_append.h"
#include "content_payload_callback.h"
#include "list_visit.h"

int FillContentConsumerEntries(struct ListCountContext *context,
                               void **entries)
{
    struct ContentAppendState state;
    struct ContentPayloadCallbackPair pair;
    int (*callback)(void *, void *);

    state.count = 0;
    state.cursor = entries;
    callback = ContentAppendCallback;
    pair.context = &state;
    pair.callback = callback;
    VisitLinkedNodes(context->unknown14, ContentPayloadCallbackAdapter, &pair);
    return state.count;
}

asm(".align 2,0");
