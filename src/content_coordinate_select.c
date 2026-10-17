#include "content_coordinate_select.h"

#include "list_accessors.h"

u32 ContentCoordinateSelectCallback(struct ListCountNode *node, void *context)
{
    struct ContentCoordinateQuery *query =
        (struct ContentCoordinateQuery *)context;
    struct ContentConsumerEntry *entry =
        (struct ContentConsumerEntry *)(unsigned long)
            GetListOwnerWord0(node);

    if (query->x == (entry->position08 >> 5)) {
        if (query->y == (entry->position0C >> 5)) {
            query->selected = entry;
            return 0;
        }
    }
    return 1;
}

asm(".align 2,0");
