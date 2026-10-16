#ifndef FFTA_LIST_SEARCH_CALLBACK_H
#define FFTA_LIST_SEARCH_CALLBACK_H

#include "types.h"

struct TreeCountNode;

struct TreeSearchContext {
    u32 key;
    struct TreeCountNode *result;
};

typedef char TreeSearchContextTargetSize[
    sizeof(void *) != 4 || sizeof(struct TreeSearchContext) == 8 ? 1 : -1];

/* Compare a tree node's second word with key; stop and retain the first match. */
int ListSearchCallback(void *value, void *context);

#endif
