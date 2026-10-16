#ifndef FFTA_ALLOCATOR_PACKED_H
#define FFTA_ALLOCATOR_PACKED_H
#include "types.h"
typedef struct AllocatorOwnerPacked {
    u16 head;
    u16 selection;
    u16 count;
} __attribute__((packed)) AllocatorOwnerPacked;
typedef struct AllocatorNodePacked {
    u8 reserved[10];
    u16 next;
} __attribute__((packed)) AllocatorNodePacked;
typedef char AllocatorOwnerPackedSizeCheck[sizeof(AllocatorOwnerPacked) == 6 ? 1 : -1];
typedef char AllocatorNodePackedSizeCheck[sizeof(AllocatorNodePacked) == 12 ? 1 : -1];
#endif
