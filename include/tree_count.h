#ifndef FFTA_TREE_COUNT_H
#define FFTA_TREE_COUNT_H

#include "types.h"
#include "container.h"
#include "list_payload.h"

/* The tree allocator reserves 24 target bytes. unknown00 is its numeric key;
   unknown04 is an opaque payload that can hold a native pointer. Four target
   words follow these data fields; the last is read and cleared by callbacks. */
struct TreeCountNode {
    u32 unknown00;
    ListPayload unknown04;
    struct TreeCountNode *parent;
    struct TreeCountNode *child_a;
    struct TreeCountNode *child_b;
    u32 flags;
};

/* Initial sequences let the old target compiler assert boundaries with sizeof. */
struct TreeCountNodeDataPrefix {
    u32 unknown00;
    ListPayload unknown04;
};

struct TreeCountNodeLinkPrefix {
    u32 unknown00;
    ListPayload unknown04;
    struct TreeCountNode *parent;
    struct TreeCountNode *child_a;
    struct TreeCountNode *child_b;
};

/* Keep the target layout explicit while using native pointers in host tests. */
typedef char TreeCountNodeTargetSize[
    sizeof(void *) != 4 || sizeof(struct TreeCountNode) == 24 ? 1 : -1];
typedef char TreeCountNodeTargetDataOffset[
    sizeof(void *) != 4 || sizeof(struct TreeCountNodeDataPrefix) == 8 ? 1 : -1];
typedef char TreeCountNodeTargetFlagsOffset[
    sizeof(void *) != 4 ||
    sizeof(struct TreeCountNodeLinkPrefix) == 20 ? 1 : -1];
typedef char TreeCountNodeHostLayout[
    sizeof(void *) != 8 ||
    (sizeof(struct TreeCountNodeDataPrefix) == 16 &&
     sizeof(struct TreeCountNodeLinkPrefix) == 40 &&
     sizeof(struct TreeCountNode) == 48) ? 1 : -1];

u32 CountTreeNodes(const struct Container *owner);

/* Target links are at 8/12/16 and flags at 20. LP64 host links are at
   16/24/32 and flags at 40 because the second word is native-width. */

#endif
