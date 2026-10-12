#ifndef FFTA_LIST_TRAVERSAL_H
#define FFTA_LIST_TRAVERSAL_H

#include "tree_count.h"

typedef int (*TreeTraversalCallback)(struct TreeCountNode *node, void *context);

/* Visit the root's binary subtree in child_a, node, child_b order. A callback
   returning zero stops the entire traversal. */
void TraverseTreeInOrder(const struct TreeCountOwner *owner,
                         TreeTraversalCallback callback, void *context);

#endif
