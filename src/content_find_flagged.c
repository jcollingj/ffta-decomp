#include "content_find_flagged.h"

#include "content_context_visit.h"
#include "content_unit_select.h"

struct ContentConsumerEntry *FindContentEntryWithUnitFlag12(
    struct ListCountContext *context)
{
    struct ContentConsumerEntry *result = 0;

    VisitContextPayloadsAt10(context, ContentUnitSelectCallback, &result);
    return result;
}

asm(".align 2, 0");
