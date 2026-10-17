#include "content_unit_select.h"

int ContentUnitSelectCallback(void *entry, void *context)
{
    struct ContentConsumerEntry *content_entry =
        (struct ContentConsumerEntry *)entry;

    if (UnitField27HasBit12(content_entry->object) == 0)
        return 1;
    *(struct ContentConsumerEntry **)context = content_entry;
    return 0;
}
