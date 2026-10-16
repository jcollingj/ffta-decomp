#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "content_globals.h"

struct ListCountContext *gContentConsumerContext;
struct ContentGlobalPointerOwner *gContentGlobalPointerOwner;

static struct ListCountContext gContexts[4];
static unsigned char gOpaqueSentinel[8];
static unsigned char gContextsBefore[sizeof(gContexts)];

static void test_consumer_context_getter(void)
{
    struct ContentConsumerContext input;
    struct ListCountContext *expected_owner;
    struct ListCountContext *actual_owner;
    unsigned char input_before[sizeof(input)];

    memset(&input, 0xA6, sizeof(input));
    memcpy(input_before, &input, sizeof(input));

    expected_owner = &gContexts[1];
    gContentConsumerContext = expected_owner;
    actual_owner = GetContentConsumerContext(0);
    assert(actual_owner == expected_owner);
    assert(gContentConsumerContext == expected_owner);
    actual_owner = GetContentConsumerContext(&input);
    assert(actual_owner == expected_owner);
    assert(gContentConsumerContext == expected_owner);
    assert(memcmp(&input, input_before, sizeof(input)) == 0);
    assert(memcmp(gContexts, gContextsBefore, sizeof(gContexts)) == 0);

    gContentConsumerContext = 0;
    actual_owner = GetContentConsumerContext(&input);
    assert(actual_owner == 0);
    assert(gContentConsumerContext == 0);
    assert(memcmp(&input, input_before, sizeof(input)) == 0);
    assert(memcmp(gContexts, gContextsBefore, sizeof(gContexts)) == 0);
}

static void test_owner_member_getter(void)
{
    struct ContentGlobalPointerOwner owner;
    struct ContentGlobalPointerOwner owner_before;
    void *expected_owner;
    void *actual_owner;

    assert(offsetof(struct ContentGlobalPointerOwner, unknown04) >= sizeof(u32));
    memset(&owner, 0x5C, sizeof(owner));
    expected_owner = &gOpaqueSentinel[3];
    owner.unknown04 = expected_owner;
    memcpy(&owner_before, &owner, sizeof(owner));
    gContentGlobalPointerOwner = &owner;

    actual_owner = GetContentContextPointerAt4();
    assert(actual_owner == expected_owner);
    assert(gContentGlobalPointerOwner == &owner);
    assert(memcmp(&owner, &owner_before, sizeof(owner)) == 0);
    assert(memcmp(gContexts, gContextsBefore, sizeof(gContexts)) == 0);

    owner.unknown04 = 0;
    memcpy(&owner_before, &owner, sizeof(owner));
    actual_owner = GetContentContextPointerAt4();
    assert(actual_owner == 0);
    assert(gContentGlobalPointerOwner == &owner);
    assert(memcmp(&owner, &owner_before, sizeof(owner)) == 0);
    assert(memcmp(gContexts, gContextsBefore, sizeof(gContexts)) == 0);
}

int main(void)
{
    memset(gContexts, 0xC3, sizeof(gContexts));
    memcpy(gContextsBefore, gContexts, sizeof(gContexts));
    assert(&gContexts[1] != &gContexts[3]);
    test_consumer_context_getter();
    test_owner_member_getter();
    return 0;
}
