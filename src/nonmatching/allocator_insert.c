#include "allocator_insert.h"

#include "allocator_recenter.h"
#include "allocator_search.h"
#include "types.h"

typedef char AllocatorInsertHeaderSizeCheck[
    (sizeof(AllocatorInsertBlockHeader) == 8) ? 1 : -1];
typedef char AllocatorInsertHeaderPrefixCheck[
    (sizeof(((AllocatorInsertBlockHeader *)0)->prefix) == 6) ? 1 : -1];

void AllocatorInsertFreeBlock(void *owner, void *block)
{
    u8 *base = (u8 *)owner;
    u8 *node = (u8 *)block;
    void *previousRaw;
    void *nextRaw;
    u16 blockOffset;
    u16 offset;
    u16 count;
    u16 size = ((AllocatorInsertBlockHeader *)block)->size;

    AllocatorFindNeighbors(base, size, &previousRaw, &nextRaw);

    blockOffset = node != 0 ? (u16)((u32)(node - base) >> 2) : 0;
    if (previousRaw != 0) {
        ((u8 *)previousRaw)[10] = (u8)blockOffset;
        ((u8 *)previousRaw)[11] = (u8)(blockOffset >> 8);
    } else {
        base[0] = (u8)blockOffset;
        base[1] = (u8)(blockOffset >> 8);
    }

    if (nextRaw != 0) {
        ((u8 *)nextRaw)[8] = (u8)blockOffset;
        ((u8 *)nextRaw)[9] = (u8)(blockOffset >> 8);
    }

    offset = previousRaw != 0 ? (u16)((u32)((u8 *)previousRaw - base) >> 2) : 0;
    node[8] = (u8)offset;
    node[9] = (u8)(offset >> 8);
    offset = nextRaw != 0 ? (u16)((u32)((u8 *)nextRaw - base) >> 2) : 0;
    node[10] = (u8)offset;
    node[11] = (u8)(offset >> 8);

    count = (u16)((u16)base[4] | ((u16)base[5] << 8));
    count++;
    base[4] = (u8)count;
    base[5] = (u8)(count >> 8);

    AllocatorRecenter(base);
}

asm(".align 2,0");
