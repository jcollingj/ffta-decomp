#ifndef FFTA_LIST_UNLINK_H
#define FFTA_LIST_UNLINK_H

#include "container.h"
#include "list_count.h"

void UnlinkAndFreeListNode(struct Container *owner,
                           struct ListCountNode *node);

#endif
