#include "allocator_recenter.h"
#include "allocator_packed.h"
void AllocatorRecenter(void *owner)
{
    AllocatorOwnerPacked *fields = (AllocatorOwnerPacked *)owner;
    AllocatorNodePacked *node;
    u16 offset;
    int remaining;
    offset = fields->head;
    node = offset ? (AllocatorNodePacked *)((u8 *)owner + (u32)offset * 4) : 0;
    if (node == 0) {
        fields->selection = 0;
    } else {
        remaining = (int)(fields->count >> 1);
        while (--remaining > 0) {
            offset = node->next;
            node = offset ? (AllocatorNodePacked *)((u8 *)owner + (u32)offset * 4) : 0;
        }
        fields->selection = node ? (u16)((u32)((u8 *)node - (u8 *)owner) >> 2) : 0;
    }
}
asm(".align 2,0");
