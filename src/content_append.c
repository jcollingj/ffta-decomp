#include "content_append.h"

int ContentAppendCallback(void *value, void *context)
{
    struct ContentAppendState *state = (struct ContentAppendState *)context;
    void **cursor = state->cursor;

    *cursor++ = value;
    state->count++;
    state->cursor = cursor;
    return 1;
}

asm(".align 2,0");
