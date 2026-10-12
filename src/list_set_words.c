#include "list_set_words.h"

void SetListWords(u32 *owner_words, u32 word4, u32 word0)
{
    owner_words[1] = word4;
    owner_words[0] = word0;
}

asm(".align 2,0");
