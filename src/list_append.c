#include "list_append.h"
#include "allocator_wrappers.h"

struct ListCountNode *AppendListNode(struct Container *owner,
                                     ListPayload payload)
{
    struct ListCountNode *node;
    struct ListCountNode *head;
    struct ListCountNode *tail;

    node = AllocatorAllocate(owner->allocator_owner, 12);
    node->unknown00 = payload;
    node->next = 0;
    node->previous = 0;

    head = owner->list_head;
    if (head == 0) {
        owner->list_head = node;
    } else {
        tail = head;
        for (;;) {
            if (tail->next == 0)
                break;
            tail = tail->next;
        }
        tail->next = node;
        node->previous = tail;
        node->next = 0;
    }

    return node;
}

asm(".align 2,0");
