#include "list_visit.h"

void VisitLinkedNodes(struct ListCountOwner *owner, ListVisitCallback callback,
                      void *context)
{
    struct ListCountNode *node = owner->head;

    while (node != 0) {
        if (callback(node, context) == 0)
            break;
        node = node->next;
    }
}

asm(".align 2,0");
