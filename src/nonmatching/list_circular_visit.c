#include "list_circular_visit.h"

void VisitCircularListFrom(struct Container *owner,
                           struct ListCountNode *start,
                           ListCircularVisitCallback callback,
                           void *context)
{
    struct ListCountNode *first = start;
    struct Container *list = owner;
    struct ListCountNode *node = first;

    do {
        if (callback(node, context) == 0)
            break;
        node = node->next;
        if (node == 0)
            node = list->list_head;
    } while (first != node);
}

asm(".align 2,0");
