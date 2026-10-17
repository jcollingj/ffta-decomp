#include "content_selection_lookup.h"

#include "list_accessors.h"

u32 ContentSelectionEqualsObject(struct ListCountNode *node, void *context)
{
    struct ContentSelectionLookupContext *lookup =
        (struct ContentSelectionLookupContext *)context;
    struct ContentConsumerEntry *entry =
        (struct ContentConsumerEntry *)(unsigned long)GetListOwnerWord0(node);

    if (entry->object == lookup->selector) {
        lookup->selected = entry;
        return 0;
    }
    return 1;
}

asm(".align 2, 0");
