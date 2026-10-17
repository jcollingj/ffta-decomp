#include "content_unit_coordinate_select.h"

#include "list_accessors.h"

u32 ContentUnitCoordinateSelectCallback(struct ListCountNode *node,
                                        void *context)
{
    struct ContentCoordinateQuery *query =
        (struct ContentCoordinateQuery *)context;
    struct ContentConsumerEntry *entry =
        (struct ContentConsumerEntry *)(unsigned long)
            GetListOwnerWord0(node);

    if (query->x == entry->object->bytesF1[5] &&
        query->y == entry->object->bytesF1[6]) {
        query->selected = entry;
        return 0;
    }
    return 1;
}

asm(".align 2,0");
