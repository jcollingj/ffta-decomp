#include "content_key_refresh.h"

#include "content_consumer.h"
#include "list_set_words.h"
#include "tree_search.h"

int ContentRefreshTreeKeyCallback(void *payload, void *opaque_owner)
{
    struct ContentConsumerEntry *entry =
        (struct ContentConsumerEntry *)payload;
    struct Container *owner = (struct Container *)opaque_owner;
    struct TreeCountNode *found;

    found = TreeSearchByWord1(owner, (ListPayload)(unsigned long)entry);
    SetListWords(found, (ListPayload)(unsigned long)entry,
                 entry->unknown24);
    return 1;
}

asm(".align 2,0");
