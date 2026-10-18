#ifndef FFTA_CONTENT_TREE_PAYLOAD_CALLBACK_H
#define FFTA_CONTENT_TREE_PAYLOAD_CALLBACK_H

#include "content_payload_callback.h"

/* Adapt a tree-node traversal callback to the content payload callback pair. */
int ContentTreePayloadCallbackAdapter(void *value, void *context);

#endif
