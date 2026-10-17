#include "content_consumer.h"
#include "content_fill.h"
#include "content_find_flagged.h"
#include "content_record.h"
#include "content_globals.h"
#include "content_selection_lookup.h"
#include "list_count_wrappers.h"
#include "list_payload.h"

extern u16 ReadCurrentContentIndex(void);
void ConsumeContentRecord(struct ContentConsumerContext *input,
                          const void *selector,
                          u16 *first_output,
                          u16 *second_output)
{
    struct ListCountContext *context;

    context = GetContentConsumerContext(input);
    if (input->flags & 0x10) {
        void *entries[13];
        signed int count;
        signed int i;
        unsigned int qualifying;
        const struct ContentRecord *record;

        record = &gContentRecords[ReadCurrentContentIndex()];
        count = FillContentConsumerEntries(context, entries);
        if (count > 12)
            count = 13;
        qualifying = 0;
        for (i = 0; i < count; i++) {
            struct ContentConsumerEntry *entry =
                (struct ContentConsumerEntry *)entries[i];
            struct UnitFieldView *object = entry->object;
            if (object != 0 && (object->half18[8] & 1))
                qualifying++;
        }
        *first_output = qualifying;
        *second_output = record->unknown20[0];
    } else {
        struct ContentConsumerEntry *selection;
        u32 additional_count;
        ListPayload helper_result;
        ListPayload has_selection;

        selection = ResolveContentConsumerSelection(context, selector);
        *first_output = (u16)(selection->value5c + 1);
        selection = FindContentEntryWithUnitFlag12(context);
        /* ListPayload is pointer-width here; fold its native representation
         * without narrowing the returned pointer to u32. */
        helper_result = (unsigned long)selection;
        has_selection = (0UL - helper_result) | helper_result;
        has_selection >>= sizeof(ListPayload) * 8 - 1;
        additional_count = CountContextListAt14(context);
        additional_count += CountContextListAt18(context);
        additional_count += has_selection;
        *second_output = (u16)additional_count;
    }
}

asm(".align 2, 0");
