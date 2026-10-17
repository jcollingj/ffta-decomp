#include "list_search_callback.h"
#include "list_accessors.h"

int ListSearchCallback(void *value, void *opaque_context)
{
    struct TreeCountNode *node = value;
    struct TreeSearchContext *context = opaque_context;
    ListPayload owner_word1 = (ListPayload)-1;
    u32 owner_word0;

    CopyListOwnerWords(node, &owner_word1, &owner_word0);
    if (context->key == owner_word1) {
        context->result = node;
        return 0;
    }
    return 1;
}
