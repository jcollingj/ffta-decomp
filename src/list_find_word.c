#include "list_find_word.h"

struct ListCountNode *FindListNodeByWord(const struct ListCountOwner *owner,
                                        u32 word)
{
    struct ListCountNode *node = owner->head;

    while (node != 0) {
        if (word == node->unknown00)
            return node;
        node = node->next;
    }
    return 0;
}

asm(".align 2,0");
