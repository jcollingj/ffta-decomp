#ifndef FFTA_TREE_REBUILD_H
#define FFTA_TREE_REBUILD_H

#include "tree_count.h"

/* Reinsert every clear-flag node under the signed numeric-key ordering. */
void RebuildTreeByNumericKey(struct Container *owner);

#endif
