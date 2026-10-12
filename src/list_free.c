#include "list_free.h"
#include "allocator_wrappers.h"

void FreeListNodes(void *owner, struct ListCountNode *node)
{
    if (node != 0) {
        if (node->next != 0)
            FreeListNodes(owner, node->next);
        AllocatorFree(owner, node);
    }
}

asm(".align 2,0");
