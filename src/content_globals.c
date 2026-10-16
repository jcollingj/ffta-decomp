#include "content_globals.h"

struct ListCountContext *GetContentConsumerContext(
    struct ContentConsumerContext *input)
{
    (void)input;
    return gContentConsumerContext;
}

void *GetContentContextPointerAt4(void)
{
    return gContentGlobalPointerOwner->unknown04;
}

asm(".align 2, 0");
