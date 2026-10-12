#ifndef FFTA_TREE_REVERSE_VISIT_H
#define FFTA_TREE_REVERSE_VISIT_H

#include "tree_count.h"

typedef int (*TreeReverseVisitCallback)(struct TreeCountNode *node, void *context);

/* Visit a rooted binary subtree in child_b, node, child_a order. */
void TraverseTreeReverseInOrder(const struct TreeCountOwner *owner,
                                TreeReverseVisitCallback callback, void *context);

#endif
