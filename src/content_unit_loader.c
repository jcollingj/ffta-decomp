#include "content_unit_loader.h"

int LoadContentUnitRecords(const struct ContentRecord *record,
                           struct UnitRecord *destination,
                           signed int limit)
{
    struct UnitRecord *cursor;
    const struct ContentSourceEntry *source;
    int count;
    int index;

    for (cursor = destination, source = record->sourceEntries,
         count = 0, index = 0;
         count < record->sourceCount && index < limit;
         source++, index++) {
        while (cursor->fields.bytes04[0] != 0)
            cursor++;

        InitializeUnitFromContentEntry(cursor, source, 0);
        count++;
    }

    return index;
}
