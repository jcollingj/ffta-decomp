#include "list_count.h"

u32 CountLinkedNodes(const struct Container *owner)
{
    const struct ListCountNode *node = owner->list_head;
    u32 count = 0;

    while (node != 0) {
        node = node->next;
        count++;
    }
    return count;
}

asm(".align 2,0");
