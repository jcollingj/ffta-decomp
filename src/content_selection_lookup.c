#include "content_selection_lookup.h"

struct ContentConsumerEntry *ResolveContentConsumerSelection(
    struct ListCountContext *context, const void *selector)
{
    struct ContentSelectionLookupContext lookup;

    lookup.selector = selector;
    lookup.selected = 0;
    VisitLinkedNodes(context->unknown10, ContentSelectionEqualsObject, &lookup);
    return lookup.selected;
}

asm(".align 2, 0");
