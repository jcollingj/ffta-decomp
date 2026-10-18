#ifndef FFTA_CONTENT_TREE_CONTEXT_VISIT_H
#define FFTA_CONTENT_TREE_CONTEXT_VISIT_H

#include "content_tree_payload_callback.h"
#include "list_count_wrappers.h"

typedef int (*ContentPayloadCallback)(void *payload, void *context);

void VisitContextTreePayloadsInOrderAt10(
    const struct ListCountContext *context,
    ContentPayloadCallback callback, void *callback_context);
void VisitContextTreePayloadsInOrderAt14(
    const struct ListCountContext *context,
    ContentPayloadCallback callback, void *callback_context);
void VisitContextTreePayloadsInOrderAt18(
    const struct ListCountContext *context,
    ContentPayloadCallback callback, void *callback_context);

void VisitContextTreePayloadsReverseAt10(
    const struct ListCountContext *context,
    ContentPayloadCallback callback, void *callback_context);
void VisitContextTreePayloadsReverseAt14(
    const struct ListCountContext *context,
    ContentPayloadCallback callback, void *callback_context);
void VisitContextTreePayloadsReverseAt18(
    const struct ListCountContext *context,
    ContentPayloadCallback callback, void *callback_context);

#endif
