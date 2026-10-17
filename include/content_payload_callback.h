#ifndef FFTA_CONTENT_PAYLOAD_CALLBACK_H
#define FFTA_CONTENT_PAYLOAD_CALLBACK_H

#include "types.h"

struct ListCountNode;

/* Original code loads context at pair offset 0 and callback at offset 4. */
struct ContentPayloadCallbackPair {
    void *context;
    int (*callback)(void *, void *);
};

typedef char ContentPayloadCallbackPairTargetSize[
    sizeof(void *) != 4 || sizeof(struct ContentPayloadCallbackPair) == 8 ? 1 : -1];

/* Adapt a list node callback to the opaque payload callback convention. */
u32 ContentPayloadCallbackAdapter(struct ListCountNode *node, void *context);

#endif
