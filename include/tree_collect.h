#ifndef FFTA_TREE_COLLECT_H
#define FFTA_TREE_COLLECT_H

#include "list_callbacks.h"
#include "list_traversal.h"
#include "tree_reverse_visit.h"

u32 TreeCollectInOrder(struct Container *owner, void **output);
u32 TreeCollectReverseInOrder(struct Container *owner, void **output);

#endif
