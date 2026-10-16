#ifndef FFTA_TREE_COUNT_H
#define FFTA_TREE_COUNT_H

#include "types.h"
#include "container.h"

/* The tree allocator reserves 24 target bytes and initializes four words
   after the two data words. The final word is also read and cleared by the
   flag callbacks. */
struct TreeCountNode {
    u32 unknown00;
    u32 unknown04;
    struct TreeCountNode *parent;
    struct TreeCountNode *child_a;
    struct TreeCountNode *child_b;
    u32 flags;
};

/* Initial sequences let the old target compiler assert boundaries with sizeof. */
struct TreeCountNodeDataPrefix {
    u32 unknown00;
    u32 unknown04;
};

struct TreeCountNodeLinkPrefix {
    u32 unknown00;
    u32 unknown04;
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
    (sizeof(struct TreeCountNodeDataPrefix) == 8 &&
     sizeof(struct TreeCountNodeLinkPrefix) == 32 &&
     sizeof(struct TreeCountNode) == 40) ? 1 : -1];

u32 CountTreeNodes(const struct Container *owner);

/* With target 32-bit pointers, links are at 8/12/16 and flags at 20. */

#endif
