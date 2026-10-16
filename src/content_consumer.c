#include "content_consumer.h"
#include "content_record.h"
#include "list_count_wrappers.h"

extern struct ListCountContext *GetContentConsumerContext(
    struct ContentConsumerContext *input);
extern signed int FillContentConsumerEntries(
    struct ListCountContext *context,
    struct ContentConsumerEntry **entries);
extern struct ContentConsumerSelection *ResolveContentConsumerSelection(
    struct ListCountContext *context, const void *selector);
extern u32 HasContentConsumerSelection(struct ListCountContext *context);
extern u16 ReadCurrentContentIndex(void);
void ConsumeContentRecord(struct ContentConsumerContext *input,
                          const void *selector,
                          u16 *first_output,
                          u16 *second_output)
{
    struct ListCountContext *context;

    context = GetContentConsumerContext(input);
    if (input->flags & 0x10) {
        struct ContentConsumerEntry *entries[13];
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
            struct ContentConsumerFlagObject *object = entries[i]->object;
            if (object != 0 && (object->flags & 1))
                qualifying++;
        }
        *first_output = qualifying;
        *second_output = record->unknown20[0];
    } else {
        struct ContentConsumerSelection *selection;
        u32 additional_count;
        u32 helper_result;
        u32 has_selection;

        selection = ResolveContentConsumerSelection(context, selector);
        *first_output = (u16)(selection->value5c + 1);
        helper_result = HasContentConsumerSelection(context);
        has_selection = (0u - helper_result) | helper_result;
        has_selection >>= 31;
        additional_count = CountContextListAt14(context);
        additional_count += CountContextListAt18(context);
        additional_count += has_selection;
        *second_output = (u16)additional_count;
    }
}

asm(".align 2, 0");
