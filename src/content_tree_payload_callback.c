#include "content_tree_payload_callback.h"

#include "list_accessors.h"

int ContentTreePayloadCallbackAdapter(void *value, void *context)
{
    struct ContentPayloadCallbackPair *pair =
        (struct ContentPayloadCallbackPair *)context;
    ListPayload payload;
    u32 numeric_key;

    CopyListOwnerWords((struct TreeCountNode *)value, &payload, &numeric_key);
    return pair->callback((void *)(unsigned long)payload, pair->context);
}

asm(".align 2,0");
