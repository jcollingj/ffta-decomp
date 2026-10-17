#include "content_context_visit.h"

#include "list_visit.h"

void VisitContextPayloadsAt10(const struct ListCountContext *context,
                              ContentPayloadCallback callback,
                              void *callback_context)
{
    struct ContentPayloadCallbackPair pair;

    pair.context = callback_context;
    pair.callback = callback;
    VisitLinkedNodes(context->unknown10, ContentPayloadCallbackAdapter, &pair);
}

void VisitContextPayloadsAt14(const struct ListCountContext *context,
                              ContentPayloadCallback callback,
                              void *callback_context)
{
    struct ContentPayloadCallbackPair pair;

    pair.context = callback_context;
    pair.callback = callback;
    VisitLinkedNodes(context->unknown14, ContentPayloadCallbackAdapter, &pair);
}

void VisitContextPayloadsAt18(const struct ListCountContext *context,
                              ContentPayloadCallback callback,
                              void *callback_context)
{
    struct ContentPayloadCallbackPair pair;

    pair.context = callback_context;
    pair.callback = callback;
    VisitLinkedNodes(context->unknown18, ContentPayloadCallbackAdapter, &pair);
}

asm(".align 2,0");
