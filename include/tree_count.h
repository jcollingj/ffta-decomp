#ifndef FFTA_TREE_COUNT_H
#define FFTA_TREE_COUNT_H

#include "types.h"

/* Only these pointer fields are established by the original loads. */
struct TreeCountNode {
    u32 unknown00;
    u32 unknown04;
    struct TreeCountNode *parent;
    struct TreeCountNode *child_a;
    struct TreeCountNode *child_b;
};

struct TreeCountOwner {
    u32 unknown00;
    u32 unknown04;
    struct TreeCountNode *root;
};

u32 CountTreeNodes(const struct TreeCountOwner *owner);

/* With target 32-bit pointers these fields land at offsets 8/12/16. The
   host harness records its native-width fixture offsets separately. */

#endif
