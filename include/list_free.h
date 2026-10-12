#ifndef FFTA_LIST_FREE_H
#define FFTA_LIST_FREE_H

#include "list_count.h"

/* Free every node in a forward list, releasing the tail before its parents. */
void FreeListNodes(void *owner, struct ListCountNode *node);

#endif
