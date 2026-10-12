#ifndef FFTA_TREE_COUNT_H
#define FFTA_TREE_COUNT_H

#include "types.h"
#include "container.h"

/* Only these pointer fields are established by the original loads. */
struct TreeCountNode {
    u32 unknown00;
    u32 unknown04;
    struct TreeCountNode *parent;
    struct TreeCountNode *child_a;
    struct TreeCountNode *child_b;
};

u32 CountTreeNodes(const struct Container *owner);

/* With target 32-bit pointers the node links land at offsets 8/12/16. */

#endif
