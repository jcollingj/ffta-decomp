#ifndef FFTA_LIST_CIRCULAR_VISIT_H
#define FFTA_LIST_CIRCULAR_VISIT_H

#include "list_count.h"

typedef u32 (*ListCircularVisitCallback)(struct ListCountNode *node,
                                         void *context);

void VisitCircularListFrom(struct Container *owner,
                           struct ListCountNode *start,
                           ListCircularVisitCallback callback,
                           void *context);

#endif
