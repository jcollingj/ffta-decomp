#ifndef FFTA_ALLOCATOR_INSERT_H
#define FFTA_ALLOCATOR_INSERT_H

#include "types.h"

typedef struct __attribute__((packed)) {
    u8 prefix[6];
    u16 size;
} AllocatorInsertBlockHeader;

/* Insert a block into the allocator's size-ordered free list. */
void AllocatorInsertFreeBlock(void *owner, void *block);

#endif
