#include "list_find_word.h"

struct ListCountNode *FindListNodeByWord(const struct Container *owner,
                                         ListPayload word)
{
    struct ListCountNode *node = owner->list_head;

    while (node != 0) {
        if (word == node->unknown00)
            return node;
        node = node->next;
    }
    return 0;
}

asm(".align 2,0");
