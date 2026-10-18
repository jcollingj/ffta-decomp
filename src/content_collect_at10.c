#include "content_collectors.h"

#include "content_append.h"
#include "content_payload_callback.h"
#include "list_visit.h"

int CollectContextListPayloadsAt10(struct ListCountContext *context,
                                   void **payloads)
{
    struct ContentAppendState state;
    struct ContentPayloadCallbackPair pair;
    int (*callback)(void *, void *);

    state.count = 0;
    state.cursor = payloads;
    callback = ContentAppendCallback;
    pair.context = &state;
    pair.callback = callback;
    VisitLinkedNodes(context->unknown10, ContentPayloadCallbackAdapter, &pair);
    return state.count;
}

asm(".align 2,0");
