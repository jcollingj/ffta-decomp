#include "list_callbacks.h"

int ListCallbackAppend(void *value, void *context)
{
    struct ListCallbackState *state = (struct ListCallbackState *)context;
    void **cursor = state->cursor;

    *cursor++ = value;
    state->count++;
    state->cursor = cursor;
    return 1;
}

#include "tree_count.h"

int ListCallbackSelectClear(void *value, void *context)
{
    struct TreeCountNode *node = value;
    void **output = context;

    if (node->flags & 1)
        return 1;
    *output = value;
    return 0;
}

int ListCallbackClearFlag0(void *value, void *context)
{
    struct TreeCountNode *node = value;

    (void)context;
    node->flags &= ~1u;
    return 1;
}

/* The original block ends with two zero alignment bytes, not an instruction. */
asm(".align 2,0");
