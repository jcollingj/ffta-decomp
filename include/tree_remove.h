#ifndef FFTA_TREE_REMOVE_H
#define FFTA_TREE_REMOVE_H

#include "tree_detach.h"

/* Detach a tree node and release it through the owner's current allocator. */
void DetachAndFreeTreeNode(struct Container *owner,
                           struct TreeCountNode *node);

#endif
