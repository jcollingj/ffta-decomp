#ifndef FFTA_CONTENT_FILL_H
#define FFTA_CONTENT_FILL_H

#include "list_count_wrappers.h"

/* Append the payloads from the context's +0x14 list to caller storage. */
int FillContentConsumerEntries(struct ListCountContext *context,
                               void **entries);

#endif
