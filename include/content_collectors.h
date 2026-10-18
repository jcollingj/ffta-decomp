#ifndef FFTA_CONTENT_COLLECTORS_H
#define FFTA_CONTENT_COLLECTORS_H

#include "list_count_wrappers.h"

int CollectContextListPayloadsAt10(struct ListCountContext *context,
                                   void **payloads);
int CollectContextListPayloadsAt18(struct ListCountContext *context,
                                   void **payloads);
int CollectContextTreePayloadsInOrderAt10(struct ListCountContext *context,
                                          void **payloads);
int CollectContextTreePayloadsReverseAt10(struct ListCountContext *context,
                                          void **payloads);
int CollectContextTreePayloadsInOrderAt14(struct ListCountContext *context,
                                          void **payloads);
int CollectContextTreePayloadsReverseAt14(struct ListCountContext *context,
                                          void **payloads);
int CollectContextTreePayloadsInOrderAt18(struct ListCountContext *context,
                                          void **payloads);
int CollectContextTreePayloadsReverseAt18(struct ListCountContext *context,
                                          void **payloads);

#endif
