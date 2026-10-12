#ifndef FFTA_ALLOCATOR_WRAPPERS_H
#define FFTA_ALLOCATOR_WRAPPERS_H

#include "types.h"

/* The internal allocator's second argument counts 4-byte units. */
void *AllocatorInternalAllocate(void *owner, u16 units);
void AllocatorInternalRelease(void *owner, void *header);

void *AllocatorAllocate(void *owner, u32 size);
void AllocatorFree(void *owner, void *allocation);
u32 AllocatorAllocationSize(void *unused, const void *allocation);

#endif
