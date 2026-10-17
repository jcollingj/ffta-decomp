#ifndef FFTA_TREE_SEARCH_H
#define FFTA_TREE_SEARCH_H

#include "list_search_callback.h"
#include "tree_count.h"

/* Return the first in-order tree node whose second word equals key. */
struct TreeCountNode *TreeSearchByWord1(const struct Container *owner,
                                        ListPayload key);

#endif
