#include "list_visit.h"

void VisitLinkedNodes(struct Container *owner, ListVisitCallback callback,
                      void *context)
{
    struct ListCountNode *node = owner->list_head;

    while (node != 0) {
        if (callback(node, context) == 0)
            break;
        node = node->next;
    }
}

asm(".align 2,0");
