#include "list_search_callback.h"
#include "list_accessors.h"

u32 ListSearchCallback(const u32 *owner_words, u32 *context_words)
{
    u32 owner_word1 = (u32)-1;
    u32 owner_word0;

    CopyListOwnerWords(owner_words, &owner_word1, &owner_word0);
    if (context_words[0] == owner_word1) {
        context_words[1] = (u32)owner_words;
        return 0;
    }
    return 1;
}
