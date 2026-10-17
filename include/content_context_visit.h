#ifndef FFTA_CONTENT_CONTEXT_VISIT_H
#define FFTA_CONTENT_CONTEXT_VISIT_H

#include "content_payload_callback.h"
#include "list_count_wrappers.h"

typedef int (*ContentPayloadCallback)(void *payload, void *context);

void VisitContextPayloadsAt10(const struct ListCountContext *context,
                              ContentPayloadCallback callback,
                              void *callback_context);
void VisitContextPayloadsAt14(const struct ListCountContext *context,
                              ContentPayloadCallback callback,
                              void *callback_context);
void VisitContextPayloadsAt18(const struct ListCountContext *context,
                              ContentPayloadCallback callback,
                              void *callback_context);

#endif
