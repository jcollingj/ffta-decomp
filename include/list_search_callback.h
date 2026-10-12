#ifndef FFTA_LIST_SEARCH_CALLBACK_H
#define FFTA_LIST_SEARCH_CALLBACK_H

#include "types.h"

/* Compares context word 0 with owner word 1; on equality stores the owner
 * address in context word 1. The observed context fields remain unnamed. */
u32 ListSearchCallback(const u32 *owner_words, u32 *context_words);

#endif
