#ifndef FFTA_CONTENT_KEY_REFRESH_H
#define FFTA_CONTENT_KEY_REFRESH_H

#include "content_payload_callback.h"
#include "list_count_wrappers.h"

int ContentRefreshTreeKeyCallback(void *payload, void *owner);

void RefreshContextTreeKeysAt10(const struct ListCountContext *context);
void RefreshContextTreeKeysAt14(const struct ListCountContext *context);
void RefreshContextTreeKeysAt18(const struct ListCountContext *context);

#endif
