#include "allocator_recenter.h"

#include "types.h"

void AllocatorRecenter(void *owner)
{
    u8 *base = (u8 *)owner;
    u8 *node;
    u16 offset;
    int remaining;

    offset = (u16)((u16)base[0] | ((u16)base[1] << 8));
    node = offset ? base + ((u32)offset << 2) : 0;
    if (node == 0) {
        base[2] = 0;
        base[3] = 0;
    } else {
        offset = (u16)((u16)base[4] | ((u16)base[5] << 8));
        remaining = (int)(offset >> 1);
        while (remaining > 1) {
            offset = (u16)((u16)node[10] | ((u16)node[11] << 8));
            node = offset ? base + ((u32)offset << 2) : 0;
            remaining--;
        }

        if (node == 0) {
            offset = 0;
        } else {
            offset = (u16)(((u8 *)node - base) / 4);
        }
        base[2] = (u8)offset;
        base[3] = (u8)(offset >> 8);
    }
}

asm(".align 2,0");
