#ifndef FFTA_LIST_VISIT_H
#define FFTA_LIST_VISIT_H

#include "list_count.h"

typedef u32 (*ListVisitCallback)(struct ListCountNode *node, void *context);

void VisitLinkedNodes(struct Container *owner, ListVisitCallback callback,
                      void *context);

#endif
