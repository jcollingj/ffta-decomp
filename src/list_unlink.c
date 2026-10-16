#include "list_unlink.h"
#include "allocator_wrappers.h"

void UnlinkAndFreeListNode(struct Container *owner,
                           struct ListCountNode *node)
{
    if (node->previous == 0)
        owner->list_head = node->next;
    else
        node->previous->next = node->next;

    if (node->next != 0)
        node->next->previous = node->previous;

    AllocatorFree(owner->allocator_owner, node);
}

asm(".align 2,0");
