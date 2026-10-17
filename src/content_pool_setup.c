#include "content_pool_setup.h"
#include "content_unit_loader.h"

void LoadAndPrepareContentUnitPool(u32 content_index,
                                   struct ListCountContext *context)
{
    u16 index = (u16)content_index;
    struct UnitRecord *unit;
    struct ContentConsumerEntry *entry;
    int i;

    LoadContentUnitRecords(&gContentRecords[index], gUnitPoolA, 12);

    unit = gUnitPoolA;
    i = 11;
    for (; i >= 0;) {
        if (unit->fields.bytes04[0] != 0) {
            entry = ResolveOrCreateContentUnitEntry(context, unit);
            entry->object->bytesF1[8] = entry->object->bytesF1[5];
            entry->object->bytesF1[9] = entry->object->bytesF1[6];
        }
        i--;
        unit++;
    }
}

asm(".align 2, 0");
