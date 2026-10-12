#ifndef FFTA_LIST_ACCESSORS_H
#define FFTA_LIST_ACCESSORS_H

#include "list_count.h"

/* The first two owner words are opaque values, not assumed pointers. */
u32 GetListOwnerWord0(const u32 *owner_words);
void CopyListOwnerWords(const u32 *owner_words, u32 *out_word4, u32 *out_word0);

/* Copies the head and subsequent nodes to output, then returns zero. */
u32 CopyListNodes(const struct Container *owner,
                  struct ListCountNode **output);

#endif
