#include "allocator_wrappers.h"

void *AllocatorAllocate(void *owner, u32 size)
{
    u32 rounded;
    u16 units;
    void *allocation;

    if (size == 0)
        return 0;
    rounded = size;
    if (size & 3)
        rounded = (size & ~3) + 4;
    units = (u16)((rounded << 14) >> 16);
    allocation = AllocatorInternalAllocate(owner, units);
    return allocation ? (u8 *)allocation + 12 : 0;
}

void AllocatorFree(void *owner, void *allocation)
{
    AllocatorInternalRelease(owner, (u8 *)allocation - 12);
}

u32 AllocatorAllocationSize(void *unused, const void *allocation)
{
    const u8 *header = (const u8 *)allocation - 12;
    u8 low = header[6];
    u8 high = header[7];

    (void)unused;
    return (((u16)high << 8) | low) * 4 - 12;
}

asm(".align 2,0");
