#ifndef FFTA_CONTENT_OTHER_SELECTION_H
#define FFTA_CONTENT_OTHER_SELECTION_H

#include "content_selection_lookup.h"

struct ContentConsumerEntry *ResolveContentSelectionAt18(
    struct ListCountContext *context, const void *selector);
struct ContentConsumerEntry *ResolveContentSelectionAt14(
    struct ListCountContext *context, const void *selector);

#endif
