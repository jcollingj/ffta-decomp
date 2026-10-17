#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "content_flag_toggle.h"
#include "content_globals.h"

struct ContentGlobalPointerOwner *gContentGlobalPointerOwner;
struct ListCountContext *gContentConsumerContext;

struct OwnerGuard {
    struct ContentGlobalPointerOwner owner;
    unsigned char after[23];
};

struct FlagVector {
    u32 initial;
    u32 zero_result;
    u32 nonzero_result;
};

static const struct FlagVector vectors[] = {
    {0x00000000, 0x00002000, 0x00000000},
    {0x00002000, 0x00002000, 0x00000000},
    {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFDFFF},
    {0x12345678, 0x12347678, 0x12345678},
    {0xA5A5B5A5, 0xA5A5B5A5, 0xA5A595A5},
};

static struct OwnerGuard owners[2];

static void FillOwner(struct OwnerGuard *guard, unsigned char fill,
                      u32 flags, void *unknown_pointer)
{
    memset(guard, fill, sizeof(*guard));
    guard->owner.unknown04 = unknown_pointer;
    guard->owner.flagsD0 = flags;
}

static void TestAllByteValuesAndFixedWholeOwnerImages(void)
{
    unsigned vector_index;
    unsigned value;

    assert(offsetof(struct ContentGlobalPointerOwner, flagsD0) ==
           offsetof(struct ContentGlobalPointerOwner, unknown08) + 0xC8);
    assert(sizeof(struct OwnerGuard) >=
           offsetof(struct OwnerGuard, owner) +
           offsetof(struct ContentGlobalPointerOwner, flagsD0) + sizeof(u32));

    for (vector_index = 0;
         vector_index < sizeof(vectors) / sizeof(vectors[0]);
         vector_index++) {
        for (value = 0; value <= 0xFF; value++) {
            struct OwnerGuard expected;
            u32 expected_flags;

            FillOwner(&owners[0], (unsigned char)(0x31 + vector_index),
                      vectors[vector_index].initial,
                      &owners[1].owner);
            memcpy(&expected, &owners[0], sizeof(expected));
            expected_flags = value == 0
                                 ? vectors[vector_index].zero_result
                                 : vectors[vector_index].nonzero_result;
            expected.owner.flagsD0 = expected_flags;
            gContentGlobalPointerOwner = &owners[0].owner;
            assert(GetContentContextPointerAt4() == &owners[1].owner);

            UpdateContentGlobalFlag2000((u8)value);

            assert(memcmp(&owners[0], &expected, sizeof(expected)) == 0);
        }
    }
}

static void TestGlobalOwnerReloadsBetweenCalls(void)
{
    struct OwnerGuard expected[2];

    FillOwner(&owners[0], 0xC3, 0xA5A5B5A5, &owners[1].owner);
    FillOwner(&owners[1], 0x6D, 0x12345678, &owners[0].owner);
    memcpy(expected, owners, sizeof(expected));

    expected[0].owner.flagsD0 = 0xA5A595A5;
    gContentGlobalPointerOwner = &owners[0].owner;
    assert(GetContentContextPointerAt4() == &owners[1].owner);
    UpdateContentGlobalFlag2000(1);
    assert(memcmp(owners, expected, sizeof(expected)) == 0);

    expected[1].owner.flagsD0 = 0x12347678;
    gContentGlobalPointerOwner = &owners[1].owner;
    assert(GetContentContextPointerAt4() == &owners[0].owner);
    UpdateContentGlobalFlag2000(0);
    assert(memcmp(owners, expected, sizeof(expected)) == 0);
}

int main(void)
{
    TestAllByteValuesAndFixedWholeOwnerImages();
    TestGlobalOwnerReloadsBetweenCalls();
    return 0;
}
