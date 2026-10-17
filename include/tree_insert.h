#ifndef FFTA_TREE_INSERT_H
#define FFTA_TREE_INSERT_H

#include "tree_count.h"

/* Insert payload under the signed key and return the newly allocated node. */
struct TreeCountNode *InsertTreeNode(struct Container *owner, ListPayload payload,
                                    int key);

#endif
