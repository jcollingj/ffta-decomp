#ifndef FFTA_TREE_REVERSE_VISIT_H
#define FFTA_TREE_REVERSE_VISIT_H

#include "tree_count.h"

typedef int (*TreeReverseVisitCallback)(void *value, void *context);

/* Visit a rooted binary subtree in child_b, node, child_a order. */
void TraverseTreeReverseInOrder(const struct Container *owner,
                                TreeReverseVisitCallback callback, void *context);

#endif
