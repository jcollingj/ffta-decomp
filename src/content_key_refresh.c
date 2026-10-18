#include "content_key_refresh.h"

#include "list_visit.h"
#include "tree_rebuild.h"

void RefreshContextTreeKeysAt10(const struct ListCountContext *context)
{
    int (*callback)(void *, void *) = ContentRefreshTreeKeyCallback;
    struct Container *owner = context->unknown10;
    struct ContentPayloadCallbackPair pair;

    pair.context = owner;
    pair.callback = callback;
    VisitLinkedNodes(owner, ContentPayloadCallbackAdapter, &pair);
    RebuildTreeByNumericKey(context->unknown10);
}

void RefreshContextTreeKeysAt14(const struct ListCountContext *context)
{
    int (*callback)(void *, void *) = ContentRefreshTreeKeyCallback;
    struct Container *owner = context->unknown14;
    struct ContentPayloadCallbackPair pair;

    pair.context = owner;
    pair.callback = callback;
    VisitLinkedNodes(owner, ContentPayloadCallbackAdapter, &pair);
    RebuildTreeByNumericKey(context->unknown14);
}

void RefreshContextTreeKeysAt18(const struct ListCountContext *context)
{
    int (*callback)(void *, void *) = ContentRefreshTreeKeyCallback;
    struct Container *owner = context->unknown18;
    struct ContentPayloadCallbackPair pair;

    pair.context = owner;
    pair.callback = callback;
    VisitLinkedNodes(owner, ContentPayloadCallbackAdapter, &pair);
    RebuildTreeByNumericKey(context->unknown18);
}

asm(".align 2,0");
