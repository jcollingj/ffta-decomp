#include "content_payload_callback.h"

#include "list_accessors.h"

u32 ContentPayloadCallbackAdapter(struct ListCountNode *node, void *context)
{
    struct ContentPayloadCallbackPair *pair =
        (struct ContentPayloadCallbackPair *)context;
    void *value = (void *)(unsigned long)GetListOwnerWord0(node);

    return pair->callback(value, pair->context);
}

asm(".align 2,0");
