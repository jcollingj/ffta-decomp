#include "allocator_unlink.h"
#include "types.h"
#include "allocator_recenter.h"

void AllocatorUnlinkFreeBlock(void *owner, void *block)
{
    u8 *base = (u8 *)owner;
    u8 *node = (u8 *)block;
    u16 previousOffset = (u16)((u16)node[8] | ((u16)node[9] << 8));
    u16 nextOffset = (u16)((u16)node[10] | ((u16)node[11] << 8));
    u8 *previous = previousOffset ? base + ((u32)previousOffset << 2) : 0;
    u8 *next = nextOffset ? base + ((u32)nextOffset << 2) : 0;
    u16 count;

    if (previous != 0) {
        previous[10] = (u8)nextOffset;
        previous[11] = (u8)(nextOffset >> 8);
    } else {
        base[0] = (u8)nextOffset;
        base[1] = (u8)(nextOffset >> 8);
    }

    if (next != 0) {
        next[8] = (u8)previousOffset;
        next[9] = (u8)(previousOffset >> 8);
    }

    node[10] = 0;
    node[11] = 0;
    node[8] = 0;
    node[9] = 0;

    count = (u16)((u16)base[4] | ((u16)base[5] << 8));
    count = (u16)(count - 1);
    base[4] = (u8)count;
    base[5] = (u8)(count >> 8);
    AllocatorRecenter(owner);
}

asm(".align 2,0");
