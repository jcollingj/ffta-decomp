#ifndef FFTA_CONTENT_UNIT_LOADER_H
#define FFTA_CONTENT_UNIT_LOADER_H

#include "content_record.h"
#include "unit_pool.h"

/* Copies counted source entries into free unit records through the copied
 * initializer. The record count and signed limit bound the source traversal. */
int LoadContentUnitRecords(const struct ContentRecord *record,
                           struct UnitRecord *destination,
                           signed int limit);

/* Diagnostic name for the copied initializer at ROM address 0x080C9644. */
void InitializeUnitFromContentEntry(struct UnitRecord *destination,
                                    const struct ContentSourceEntry *source_entry,
                                    u32 selector);

#endif
