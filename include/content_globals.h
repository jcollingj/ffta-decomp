#ifndef FFTA_CONTENT_GLOBALS_H
#define FFTA_CONTENT_GLOBALS_H

#include "content_consumer.h"
#include "list_count_wrappers.h"

/* RAM 0x0200F4B0 contains a native pointer to the consumer context. */
extern struct ListCountContext *gContentConsumerContext;

/* RAM 0x0200F4A8 points to a partially recovered global content owner. */
struct ContentGlobalPointerOwner {
    u32 unknown00;
    void *unknown04;
    u8 unknown08[0xC8];
    u32 flagsD0;
};

extern struct ContentGlobalPointerOwner *gContentGlobalPointerOwner;

struct ListCountContext *GetContentConsumerContext(
    struct ContentConsumerContext *input);
void *GetContentContextPointerAt4(void);
void ClearContentGlobalFlag800AndSet1000(void);
void AddContentGlobalFlags8800(void);
void ClearContentGlobalFlags8800(void);

/* The known target prefix extends through the word at +0xD0. */
typedef char ContentGlobalPointerOwnerTargetSize[
    sizeof(void *) != 4 ||
    sizeof(struct ContentGlobalPointerOwner) == 0xD4 ? 1 : -1];

#endif
