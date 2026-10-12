#include "list_accessors.h"

/* The two leading list words are intentionally kept opaque. */
u32 GetListOwnerWord0(const u32 *owner_words)
{
    return owner_words[0];
}

void CopyListOwnerWords(const u32 *owner_words, u32 *out_word4, u32 *out_word0)
{
    *out_word4 = owner_words[1];
    *out_word0 = owner_words[0];
}

u32 CopyListNodes(const struct ListCountOwner *owner,
                  struct ListCountNode **output)
{
    const struct ListCountNode *node = owner->head;

    while (node != 0) {
        *output++ = (struct ListCountNode *)node;
        node = node->next;
    }
    return 0;
}

/* Preserve the original two-byte zero-filled end padding of this block. */
asm(".align 2, 0");
