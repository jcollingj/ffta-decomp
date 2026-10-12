#ifndef FFTA_LIST_CALLBACKS_H
#define FFTA_LIST_CALLBACKS_H

#include "types.h"

/* This callback state has the two words observed in the original routine. */
struct ListCallbackState {
    void **cursor;
    u32 count;
};

/* Target pointers occupy one word; the host harness uses native host pointers. */
typedef char ListCallbackStateTargetSize[
    sizeof(void *) != 4 || sizeof(struct ListCallbackState) == 8 ? 1 : -1];
typedef char ListCallbackStateHostSize[
    sizeof(void *) != 8 || sizeof(struct ListCallbackState) == 16 ? 1 : -1];

int ListCallbackAppend(void *value, struct ListCallbackState *state);
int ListCallbackSelectClear(const u32 *node_words, const u32 **output);
int ListCallbackClearFlag0(u32 *node_words);

#endif
