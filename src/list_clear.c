#include "list_clear.h"
#include "list_free.h"
#include "allocator_wrappers.h"

void ClearContainerList(struct Container *owner)
{
    void *allocator_owner = owner->allocator_owner;
    struct ListCountNode *head = owner->list_head;

    if (head != 0) {
        if (head->next != 0)
            FreeListNodes(allocator_owner, head->next);
        AllocatorFree(allocator_owner, head);
    }
    owner->list_head = 0;
}

asm(".align 2,0");
