#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "content_globals.h"

struct ContentGlobalPointerOwner *gContentGlobalPointerOwner;
struct ListCountContext *gContentConsumerContext;

struct OwnerGuard {
    struct ContentGlobalPointerOwner owner;
    unsigned char after[19];
};

static struct OwnerGuard guards[2];
static unsigned char pointee[13];

struct Vector {
    u32 input;
    u32 clear_and_set;
    u32 add;
    u32 clear;
};

/* Independent fixed outputs, covering each bit and mask boundaries. */
static const struct Vector vectors[] = {
    {0x00000001, 0x00001001, 0x00008801, 0x00000001},
    {0x00000002, 0x00001002, 0x00008802, 0x00000002},
    {0x00000004, 0x00001004, 0x00008804, 0x00000004},
    {0x00000008, 0x00001008, 0x00008808, 0x00000008},
    {0x00000010, 0x00001010, 0x00008810, 0x00000010},
    {0x00000020, 0x00001020, 0x00008820, 0x00000020},
    {0x00000040, 0x00001040, 0x00008840, 0x00000040},
    {0x00000080, 0x00001080, 0x00008880, 0x00000080},
    {0x00000100, 0x00001100, 0x00008900, 0x00000100},
    {0x00000200, 0x00001200, 0x00008A00, 0x00000200},
    {0x00000400, 0x00001400, 0x00008C00, 0x00000400},
    {0x00000800, 0x00001000, 0x00008800, 0x00000000},
    {0x00001000, 0x00001000, 0x00009800, 0x00001000},
    {0x00002000, 0x00003000, 0x0000A800, 0x00002000},
    {0x00004000, 0x00005000, 0x0000C800, 0x00004000},
    {0x00008000, 0x00009000, 0x00008800, 0x00000000},
    {0x00010000, 0x00011000, 0x00018800, 0x00010000},
    {0x00020000, 0x00021000, 0x00028800, 0x00020000},
    {0x00040000, 0x00041000, 0x00048800, 0x00040000},
    {0x00080000, 0x00081000, 0x00088800, 0x00080000},
    {0x00100000, 0x00101000, 0x00108800, 0x00100000},
    {0x00200000, 0x00201000, 0x00208800, 0x00200000},
    {0x00400000, 0x00401000, 0x00408800, 0x00400000},
    {0x00800000, 0x00801000, 0x00808800, 0x00800000},
    {0x01000000, 0x01001000, 0x01008800, 0x01000000},
    {0x02000000, 0x02001000, 0x02008800, 0x02000000},
    {0x04000000, 0x04001000, 0x04008800, 0x04000000},
    {0x08000000, 0x08001000, 0x08008800, 0x08000000},
    {0x10000000, 0x10001000, 0x10008800, 0x10000000},
    {0x20000000, 0x20001000, 0x20008800, 0x20000000},
    {0x40000000, 0x40001000, 0x40008800, 0x40000000},
    {0x80000000, 0x80001000, 0x80008800, 0x80000000},
    {0x00000000, 0x00001000, 0x00008800, 0x00000000},
    {0xFFFFFFFF, 0xFFFFF7FF, 0xFFFFFFFF, 0xFFFF77FF},
    {0x12345678, 0x12345678, 0x1234DE78, 0x12345678},
    {0xA5A58000, 0xA5A59000, 0xA5A58800, 0xA5A50000},
};

static void FillOwner(struct OwnerGuard *guard, unsigned char fill,
                      u32 flags, void *pointer)
{
    memset(guard, fill, sizeof(*guard));
    guard->owner.unknown04 = pointer;
    guard->owner.flagsD0 = flags;
}

static void CheckMutation(struct OwnerGuard *guard,
                          void (*operation)(void), u32 expected_flags)
{
    struct OwnerGuard expected;

    memcpy(&expected, guard, sizeof(expected));
    expected.owner.flagsD0 = expected_flags;
    gContentGlobalPointerOwner = &guard->owner;
    operation();
    assert(memcmp(guard, &expected, sizeof(expected)) == 0);
}

static void TestLayoutAndFixedVectors(void)
{
    unsigned i;

    assert(offsetof(struct ContentGlobalPointerOwner, unknown04) >= sizeof(u32));
    assert(offsetof(struct ContentGlobalPointerOwner, unknown08) ==
           offsetof(struct ContentGlobalPointerOwner, unknown04) + sizeof(void *));
    assert(offsetof(struct ContentGlobalPointerOwner, flagsD0) ==
           offsetof(struct ContentGlobalPointerOwner, unknown08) + 0xC8);
    assert(sizeof(struct ContentGlobalPointerOwner) >=
           offsetof(struct ContentGlobalPointerOwner, flagsD0) + sizeof(u32));
    memset(pointee, 0x5A, sizeof(pointee));

    for (i = 0; i < sizeof(vectors) / sizeof(vectors[0]); i++) {
        FillOwner(&guards[0], (unsigned char)(0x31 + i),
                  vectors[i].input, pointee);
        CheckMutation(&guards[0], ClearContentGlobalFlag800AndSet1000,
                      vectors[i].clear_and_set);

        FillOwner(&guards[0], (unsigned char)(0x71 + i),
                  vectors[i].input, pointee);
        CheckMutation(&guards[0], AddContentGlobalFlags8800, vectors[i].add);

        FillOwner(&guards[0], (unsigned char)(0xA1 + i),
                  vectors[i].input, pointee);
        CheckMutation(&guards[0], ClearContentGlobalFlags8800,
                      vectors[i].clear);
    }
}

static void TestSequenceReloadAndGetterComposition(void)
{
    struct OwnerGuard expected;
    unsigned char owner0_before[sizeof(guards[0])];
    unsigned char owner1_before[sizeof(guards[1])];

    FillOwner(&guards[0], 0xC3, 0x12345678, pointee);
    FillOwner(&guards[1], 0x6D, 0x80000800, &pointee[3]);
    memcpy(owner0_before, &guards[0], sizeof(guards[0]));
    memcpy(owner1_before, &guards[1], sizeof(guards[1]));

    gContentGlobalPointerOwner = &guards[0].owner;
    assert(GetContentContextPointerAt4() == pointee);
    memcpy(&expected, &guards[0], sizeof(expected));
    expected.owner.flagsD0 = 0x12345678;
    ClearContentGlobalFlag800AndSet1000();
    assert(memcmp(&guards[0], &expected, sizeof(expected)) == 0);
    expected.owner.flagsD0 = 0x1234DE78;
    AddContentGlobalFlags8800();
    assert(memcmp(&guards[0], &expected, sizeof(expected)) == 0);
    expected.owner.flagsD0 = 0x12345678;
    ClearContentGlobalFlags8800();
    assert(memcmp(&guards[0], &expected, sizeof(expected)) == 0);

    /* The next call must use the newly loaded global owner. */
    gContentGlobalPointerOwner = &guards[1].owner;
    assert(GetContentContextPointerAt4() == &pointee[3]);
    memcpy(&expected, &guards[1], sizeof(expected));
    expected.owner.flagsD0 = 0x80001000;
    ClearContentGlobalFlag800AndSet1000();
    assert(memcmp(&guards[1], &expected, sizeof(expected)) == 0);
    assert(memcmp(&guards[0], owner0_before, sizeof(guards[0])) == 0);
    assert(memcmp(&guards[1], owner1_before, sizeof(guards[1])) != 0);
    assert(guards[0].owner.unknown04 == pointee);
    assert(guards[1].owner.unknown04 == &pointee[3]);
}

int main(void)
{
    TestLayoutAndFixedVectors();
    TestSequenceReloadAndGetterComposition();
    return 0;
}
