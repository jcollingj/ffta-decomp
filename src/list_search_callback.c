#include "list_search_callback.h"
#include "list_accessors.h"

int ListSearchCallback(void *value, void *opaque_context)
{
    struct TreeCountNode *node = value;
    struct TreeSearchContext *context = opaque_context;
    u32 owner_word1 = (u32)-1;
    u32 owner_word0;

    CopyListOwnerWords((const u32 *)value, &owner_word1, &owner_word0);
    if (context->key == owner_word1) {
        context->result = node;
        return 0;
    }
    return 1;
}
