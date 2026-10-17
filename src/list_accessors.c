#include "list_accessors.h"

/* The payload is an opaque native-width word shared by list clients. */
ListPayload GetListOwnerWord0(const struct ListCountNode *node)
{
    return node->unknown00;
}

void CopyListOwnerWords(const u32 *owner_words, u32 *out_word4, u32 *out_word0)
{
    *out_word4 = owner_words[1];
    *out_word0 = owner_words[0];
}

u32 CopyListNodes(const struct Container *owner,
                  struct ListCountNode **output)
{
    const struct ListCountNode *node = owner->list_head;

    while (node != 0) {
        *output++ = (struct ListCountNode *)node;
        node = node->next;
    }
    return 0;
}

/* Preserve the original two-byte zero-filled end padding of this block. */
asm(".align 2, 0");
