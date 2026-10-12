#include "list_callbacks.h"

int ListCallbackAppend(void *value, struct ListCallbackState *state)
{
    void **cursor = state->cursor;

    *cursor++ = value;
    state->count++;
    state->cursor = cursor;
    return 1;
}

int ListCallbackSelectClear(const u32 *node_words, const u32 **output)
{
    if (node_words[5] & 1)
        return 1;
    *output = node_words;
    return 0;
}

int ListCallbackClearFlag0(u32 *node_words)
{
    node_words[5] &= ~1u;
    return 1;
}

/* The original block ends with two zero alignment bytes, not an instruction. */
asm(".align 2,0");
