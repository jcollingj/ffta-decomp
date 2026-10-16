#include "tree_remove.h"

#include "allocator_wrappers.h"

void DetachAndFreeTreeNode(struct Container *owner,
                           struct TreeCountNode *node)
{
    DetachTreeNode(owner, node);
    AllocatorFree(owner->allocator_owner, node);
}

#ifndef TREE_REMOVE_HOST_TEST
asm(".align 2,0");
#endif
