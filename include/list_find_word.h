#ifndef FFTA_LIST_FIND_WORD_H
#define FFTA_LIST_FIND_WORD_H

#include "list_count.h"

/* Returns the first node whose word at offset zero equals word. */
struct ListCountNode *FindListNodeByWord(const struct Container *owner,
                                         ListPayload word);

#endif
