#include "allocator_search.h"

void AllocatorFindNeighbors(void *owner, u16 requested, void **outA, void **outB)
{
    u8 *base;
    u8 *node3;
    u8 *node2;
    u16 offset;

    base = (u8 *)owner;
    *outB = 0;
    *outA = 0;
    offset = (u16)((u16)base[2] | ((u16)base[3] << 8));
    if (offset == 0)
        return;
    node3 = base + (offset << 2);
    if (requested > (u16)((u16)node3[6] | ((u16)node3[7] << 8)))
        goto forward;

    node2 = node3;
backward:
    offset = (u16)((u16)node2[8] | ((u16)node2[9] << 8));
    node3 = offset ? base + (offset << 2) : 0;
    if (node3 == 0)
        goto output;
    if (requested <= (u16)((u16)node3[6] | ((u16)node3[7] << 8))) {
        node2 = node3;
        goto backward;
    }
    if (requested <= (u16)((u16)node2[6] | ((u16)node2[7] << 8)))
        goto output;
    /* ROM falls through to this same backward step when both compares fail. */
    node2 = node3;
    goto backward;

forward:
    for (;;) {
        offset = (u16)((u16)node3[10] | ((u16)node3[11] << 8));
        node2 = offset ? base + (offset << 2) : 0;
        if (node2 == 0)
            break;
        if (requested <= (u16)((u16)node3[6] | ((u16)node3[7] << 8))) {
            node3 = node2;
            continue;
        }
        if (requested <= (u16)((u16)node2[6] | ((u16)node2[7] << 8)))
            break;
        node3 = node2;
    }

output:
    *outA = node3;
    *outB = node2;
}

asm(".align 2,0");
