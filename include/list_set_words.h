#ifndef FFTA_LIST_SET_WORDS_H
#define FFTA_LIST_SET_WORDS_H

#include "types.h"

/* Stores word4 at owner offset four, then word0 at owner offset zero. */
void SetListWords(u32 *owner_words, u32 word4, u32 word0);

#endif
