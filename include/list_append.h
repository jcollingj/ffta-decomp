#ifndef FFTA_LIST_APPEND_H
#define FFTA_LIST_APPEND_H

#include "container.h"
#include "list_count.h"

struct ListCountNode *AppendListNode(struct Container *owner,
                                     ListPayload payload);

#endif
