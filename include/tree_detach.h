#ifndef FFTA_TREE_DETACH_H
#define FFTA_TREE_DETACH_H

#include "tree_count.h"

/* Remove node from owner and reinsert its former child subtrees by key. */
void DetachTreeNode(struct Container *owner, struct TreeCountNode *node);

#endif
