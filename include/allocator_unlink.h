#ifndef FFTA_ALLOCATOR_UNLINK_H
#define FFTA_ALLOCATOR_UNLINK_H

/* Remove a block from an allocator's size-ordered free list. */
void AllocatorUnlinkFreeBlock(void *owner, void *block);

#endif
