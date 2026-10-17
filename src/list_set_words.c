#include "list_set_words.h"

void SetListWords(struct TreeCountNode *owner_words, ListPayload word4,
                  u32 word0)
{
    owner_words->unknown04 = word4;
    owner_words->unknown00 = word0;
}

asm(".align 2,0");
