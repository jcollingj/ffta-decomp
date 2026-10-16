#ifndef FFTA_CONTENT_APPEND_H
#define FFTA_CONTENT_APPEND_H

#include "types.h"

/* The native output state begins with a count and then an output cursor. */
struct ContentAppendState {
    u32 count;
    void **cursor;
};

typedef char ContentAppendTargetStateSize[
    sizeof(void *) != 4 || sizeof(struct ContentAppendState) == 8 ? 1 : -1];

/* Generic value/context callback used by the content callback pair. */
int ContentAppendCallback(void *value, void *context);

#endif
