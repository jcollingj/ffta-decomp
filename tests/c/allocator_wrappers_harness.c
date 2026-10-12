#include "allocator_wrappers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void *sReturnedBase;
static void *sRecordedOwner;
static void *sRecordedHeader;
static u16 sRecordedUnits;
static unsigned sAllocateCalls;
static unsigned sReleaseCalls;

void *AllocatorInternalAllocate(void *owner, u16 units)
{
    ++sAllocateCalls;
    sRecordedOwner = owner;
    sRecordedUnits = units;
    return sReturnedBase;
}

void AllocatorInternalRelease(void *owner, void *header)
{
    ++sReleaseCalls;
    sRecordedOwner = owner;
    sRecordedHeader = header;
}

static void fail(const char *message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}

static void test_allocate(void)
{
    u8 owner[4];
    u8 ownerBefore[4];
    u8 backing[32];
    u8 backingBefore[32];
    struct FixedAllocationCase {
        u32 size;
        u16 units;
    };
    static const struct FixedAllocationCase cases[] = {
        { 1, 1 }, { 2, 1 }, { 3, 1 }, { 4, 1 },
        { 5, 2 }, { 6, 2 }, { 7, 2 }, { 8, 2 },
        { 9, 3 }, { 10, 3 }, { 11, 3 }, { 12, 3 },
        { 0xFFFF, 0x4000 }, { 0x10000, 0x4000 },
        { 0x3FFFC, 0xFFFF }, { 0x3FFFF, 0 },
        { 0x40000, 0 }, { 0x40001, 1 },
        { 0xFFFFFFFC, 0xFFFF }, { 0xFFFFFFFD, 0 },
        { 0xFFFFFFFE, 0 }, { 0xFFFFFFFF, 0 }
    };
    unsigned i;

    memset(owner, 0xB6, sizeof(owner));
    memset(backing, 0xC3, sizeof(backing));
    memcpy(ownerBefore, owner, sizeof(owner));
    memcpy(backingBefore, backing, sizeof(backing));
    sReturnedBase = backing;
    sAllocateCalls = 0;
    if (AllocatorAllocate(owner, 0) != 0 || sAllocateCalls != 0)
        fail("zero-size request did not return before allocator call");
    if (memcmp(ownerBefore, owner, sizeof(owner)) != 0 ||
        memcmp(backingBefore, backing, sizeof(backing)) != 0)
        fail("zero-size request changed caller state");

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        void *result = AllocatorAllocate(owner, cases[i].size);
        if (sAllocateCalls != i + 1 || sRecordedOwner != owner ||
            sRecordedUnits != cases[i].units)
            fail("allocation callback trace or narrowed units mismatch");
        if (result != backing + 12)
            fail("successful allocation did not return base plus header size");
        if (memcmp(ownerBefore, owner, sizeof(owner)) != 0 ||
            memcmp(backingBefore, backing, sizeof(backing)) != 0)
            fail("allocation wrapper changed caller state");
    }

    sReturnedBase = 0;
    if (AllocatorAllocate(owner, 3) != 0 || sAllocateCalls != i + 1 ||
        sRecordedOwner != owner || sRecordedUnits != 1)
        fail("null allocator result handling mismatch");
    if (memcmp(ownerBefore, owner, sizeof(owner)) != 0 ||
        memcmp(backingBefore, backing, sizeof(backing)) != 0)
        fail("failed allocation changed caller state");
}

static void test_free(void)
{
    u8 owner[4];
    u8 ownerBefore[4];
    u8 storage[32];
    u8 storageBefore[32];
    memset(owner, 0x4B, sizeof(owner));
    memset(storage, 0xD7, sizeof(storage));
    memcpy(ownerBefore, owner, sizeof(owner));
    memcpy(storageBefore, storage, sizeof(storage));
    sReleaseCalls = 0;
    AllocatorFree(owner, storage + 12);
    if (sReleaseCalls != 1 || sRecordedOwner != owner ||
        sRecordedHeader != storage)
        fail("release callback owner/header mismatch");
    if (memcmp(ownerBefore, owner, sizeof(owner)) != 0 ||
        memcmp(storageBefore, storage, sizeof(storage)) != 0)
        fail("free wrapper changed caller state");
}

static void test_size(void)
{
    u8 storage[32];
    u8 before[32];
    unsigned value;

    memset(storage, 0xA5, sizeof(storage));
    storage[6] = 0x39;
    storage[7] = 0xC7;
    storage[12 + 6] = 0xE2;
    storage[12 + 7] = 0x19;
    memcpy(before, storage, sizeof(storage));
    if (AllocatorAllocationSize((void *)0x1234, storage + 12) !=
        ((u32)0xC739 * 4 - 12))
        fail("size helper did not use little-endian header bytes 6 and 7");
    if (memcmp(before, storage, sizeof(storage)) != 0)
        fail("size helper changed allocation/header bytes");

    for (value = 0; value <= 0xFFFF; ++value) {
        storage[6] = (u8)value;
        storage[7] = (u8)(value >> 8);
        storage[12 + 6] = (u8)~value;
        storage[12 + 7] = (u8)~(value >> 8);
        memcpy(before, storage, sizeof(storage));
        if (AllocatorAllocationSize((void *)0xDEAD, storage + 12) !=
            ((u32)value * 4 - 12))
            fail("size helper failed for a 16-bit little-endian unit value");
        if (memcmp(before, storage, sizeof(storage)) != 0)
            fail("size helper changed allocation/header bytes");
    }
}

int main(void)
{
    test_allocate();
    test_free();
    test_size();
    puts("allocator wrapper behavior checks passed");
    return 0;
}
