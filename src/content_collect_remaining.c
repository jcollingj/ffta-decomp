#include "content_collectors.h"

#include "content_append.h"
#include "content_payload_callback.h"
#include "content_tree_payload_callback.h"
#include "list_traversal.h"
#include "list_visit.h"
#include "tree_reverse_visit.h"

int CollectContextListPayloadsAt18(struct ListCountContext *context,
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
    VisitLinkedNodes(context->unknown18, ContentPayloadCallbackAdapter, &pair);
    return state.count;
}

int CollectContextTreePayloadsInOrderAt10(struct ListCountContext *context,
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
    TraverseTreeInOrder(context->unknown10,
                        ContentTreePayloadCallbackAdapter, &pair);
    return state.count;
}

int CollectContextTreePayloadsReverseAt10(struct ListCountContext *context,
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
    TraverseTreeReverseInOrder(context->unknown10,
                               ContentTreePayloadCallbackAdapter, &pair);
    return state.count;
}

int CollectContextTreePayloadsInOrderAt14(struct ListCountContext *context,
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
    TraverseTreeInOrder(context->unknown14,
                        ContentTreePayloadCallbackAdapter, &pair);
    return state.count;
}

int CollectContextTreePayloadsReverseAt14(struct ListCountContext *context,
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
    TraverseTreeReverseInOrder(context->unknown14,
                               ContentTreePayloadCallbackAdapter, &pair);
    return state.count;
}

int CollectContextTreePayloadsInOrderAt18(struct ListCountContext *context,
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
    TraverseTreeInOrder(context->unknown18,
                        ContentTreePayloadCallbackAdapter, &pair);
    return state.count;
}

int CollectContextTreePayloadsReverseAt18(struct ListCountContext *context,
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
    TraverseTreeReverseInOrder(context->unknown18,
                               ContentTreePayloadCallbackAdapter, &pair);
    return state.count;
}

asm(".align 2,0");
