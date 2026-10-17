#include "content_other_selection.h"

struct ContentConsumerEntry *ResolveContentSelectionAt18(
    struct ListCountContext *context, const void *selector)
{
    struct ContentSelectionLookupContext lookup;

    lookup.selector = selector;
    lookup.selected = 0;
    VisitLinkedNodes(context->unknown18, ContentSelectionEqualsObject, &lookup);
    return lookup.selected;
}

struct ContentConsumerEntry *ResolveContentSelectionAt14(
    struct ListCountContext *context, const void *selector)
{
    struct ContentSelectionLookupContext lookup;

    lookup.selector = selector;
    lookup.selected = 0;
    VisitLinkedNodes(context->unknown14, ContentSelectionEqualsObject, &lookup);
    return lookup.selected;
}

asm(".align 2, 0");
