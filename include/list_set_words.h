#ifndef FFTA_LIST_SET_WORDS_H
#define FFTA_LIST_SET_WORDS_H

#include "tree_count.h"

/* Stores the native payload at offset four, then the numeric key at zero. */
void SetListWords(struct TreeCountNode *owner_words, ListPayload word4,
                  u32 word0);

#endif
