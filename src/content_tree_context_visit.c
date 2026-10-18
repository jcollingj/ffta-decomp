#include "content_tree_context_visit.h"

#include "list_traversal.h"
#include "tree_reverse_visit.h"

void VisitContextTreePayloadsInOrderAt10(
    const struct ListCountContext *context,
    ContentPayloadCallback callback, void *callback_context)
{
    struct ContentPayloadCallbackPair pair;

    pair.context = callback_context;
    pair.callback = callback;
    TraverseTreeInOrder(context->unknown10, ContentTreePayloadCallbackAdapter,
                        &pair);
}

void VisitContextTreePayloadsInOrderAt14(
    const struct ListCountContext *context,
    ContentPayloadCallback callback, void *callback_context)
{
    struct ContentPayloadCallbackPair pair;

    pair.context = callback_context;
    pair.callback = callback;
    TraverseTreeInOrder(context->unknown14, ContentTreePayloadCallbackAdapter,
                        &pair);
}

void VisitContextTreePayloadsInOrderAt18(
    const struct ListCountContext *context,
    ContentPayloadCallback callback, void *callback_context)
{
    struct ContentPayloadCallbackPair pair;

    pair.context = callback_context;
    pair.callback = callback;
    TraverseTreeInOrder(context->unknown18, ContentTreePayloadCallbackAdapter,
                        &pair);
}

void VisitContextTreePayloadsReverseAt10(
    const struct ListCountContext *context,
    ContentPayloadCallback callback, void *callback_context)
{
    struct ContentPayloadCallbackPair pair;

    pair.context = callback_context;
    pair.callback = callback;
    TraverseTreeReverseInOrder(context->unknown10,
                               ContentTreePayloadCallbackAdapter, &pair);
}

void VisitContextTreePayloadsReverseAt14(
    const struct ListCountContext *context,
    ContentPayloadCallback callback, void *callback_context)
{
    struct ContentPayloadCallbackPair pair;

    pair.context = callback_context;
    pair.callback = callback;
    TraverseTreeReverseInOrder(context->unknown14,
                               ContentTreePayloadCallbackAdapter, &pair);
}

void VisitContextTreePayloadsReverseAt18(
    const struct ListCountContext *context,
    ContentPayloadCallback callback, void *callback_context)
{
    struct ContentPayloadCallbackPair pair;

    pair.context = callback_context;
    pair.callback = callback;
    TraverseTreeReverseInOrder(context->unknown18,
                               ContentTreePayloadCallbackAdapter, &pair);
}

asm(".align 2,0");
