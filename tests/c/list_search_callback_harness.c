#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "../../include/list_search_callback.h"
#include "../../src/list_search_callback.c"

static u32 HostOwnerWord(const u32 *owner_words)
{
    return (u32)(uintptr_t)owner_words;
}

static void TestMismatchLeavesContextUnchanged(void)
{
    static const u32 owner[2] = {0xA1B2C3D4, 0x13579BDF};
    u32 context[3] = {0x2468ACE0, 0xDEADBEEF, 0xCAFEBABE};
    const u32 before[3] = {0x2468ACE0, 0xDEADBEEF, 0xCAFEBABE};

    assert(ListSearchCallback(owner, context) == 1);
    assert(memcmp(context, before, sizeof(context)) == 0);
}

static void TestMatchStoresOwnerAddress(void)
{
    static const u32 owner[2] = {0x01020304, 0x89ABCDEF};
    u32 context[3] = {0x89ABCDEF, 0xDEADBEEF, 0x76543210};

    assert(ListSearchCallback(owner, context) == 0);
    assert(context[0] == 0x89ABCDEF);
    assert(context[1] == HostOwnerWord(owner));
    assert(context[2] == 0x76543210);
}

static void TestAliasedOwnerAndContext(void)
{
    u32 words[3] = {0xCAFEBABE, 0xCAFEBABE, 0xDEADBEEF};
    u32 mismatch[3] = {0x13572468, 0x11223344, 0x55667788};
    const u32 mismatch_before[3] = {0x13572468, 0x11223344, 0x55667788};

    /* The compare is against owner word 1 copied before context word 1 is
     * overwritten; same-base aliasing therefore succeeds. */
    assert(ListSearchCallback(words, words) == 0);
    assert(words[0] == 0xCAFEBABE);
    assert(words[1] == HostOwnerWord(words));
    assert(words[2] == 0xDEADBEEF);

    /* Mismatch remains a no-write path even when owner and context alias. */
    assert(ListSearchCallback(mismatch, mismatch) == 1);
    assert(memcmp(mismatch, mismatch_before, sizeof(mismatch)) == 0);
}

int main(void)
{
    TestMismatchLeavesContextUnchanged();
    TestMatchStoresOwnerAddress();
    TestAliasedOwnerAndContext();
    return 0;
}
