#ifndef FFTA_CONTENT_GLOBALS_H
#define FFTA_CONTENT_GLOBALS_H

#include "content_consumer.h"
#include "list_count_wrappers.h"

/* RAM 0x0200F4B0 contains a native pointer to the consumer context. */
extern struct ListCountContext *gContentConsumerContext;

/* RAM 0x0200F4A8 points to an owner whose pointer at target offset +4 is read. */
struct ContentGlobalPointerOwner {
    u32 unknown00;
    void *unknown04;
};

extern struct ContentGlobalPointerOwner *gContentGlobalPointerOwner;

struct ListCountContext *GetContentConsumerContext(
    struct ContentConsumerContext *input);
void *GetContentContextPointerAt4(void);

/* On GBA, the native pointer follows the first word at target offset +4. */
typedef char ContentGlobalPointerOwnerTargetSize[
    sizeof(void *) != 4 ||
    sizeof(struct ContentGlobalPointerOwner) == 8 ? 1 : -1];

#endif
