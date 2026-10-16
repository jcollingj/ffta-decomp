#ifndef FFTA_TREE_CLEAR_H
#define FFTA_TREE_CLEAR_H

#include "container.h"

/* Release the nodes in a tree, then clear its owner root. */
void FreeTreeNodes(struct Container *owner);

#endif
