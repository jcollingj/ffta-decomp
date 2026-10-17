#ifndef FFTA_LIST_ACCESSORS_H
#define FFTA_LIST_ACCESSORS_H

#include "tree_count.h"
#include "list_count.h"

/* Reads the generic payload from a list node. */
ListPayload GetListOwnerWord0(const struct ListCountNode *node);
/* Copies a tree node's payload, then numeric key, preserving store order. */
void CopyListOwnerWords(const struct TreeCountNode *owner_words,
                        ListPayload *out_word4, u32 *out_word0);

/* Copies the head and subsequent nodes to output, then returns zero. */
u32 CopyListNodes(const struct Container *owner,
                  struct ListCountNode **output);

#endif
