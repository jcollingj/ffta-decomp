#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "../../include/list_accessors.h"
#include "../../src/list_accessors.c"

static void TestWord0Getter(void)
{
    struct ListCountNode node;
    struct ListCountNode before;

    memset(&node, 0x5A, sizeof(node));
    node.unknown00 = 0x13579BDFUL;
    before = node;
    assert(GetListOwnerWord0(&node) == 0x13579BDFUL);
    assert(memcmp(&before, &node, sizeof(node)) == 0);
}

static void TestWordCopyAliases(void)
{
    unsigned out1_alias;
    unsigned out2_alias;

    /* 0 means a separate output; 1/2 mean input word 0/word 4. */
    for (out1_alias = 0; out1_alias <= 2; out1_alias++) {
        for (out2_alias = 0; out2_alias <= 2; out2_alias++) {
            u32 words[2] = {0x11223344, 0xA1B2C3D4};
            u32 expected_words[2] = {0x11223344, 0xA1B2C3D4};
            u32 separate1 = 0xDEADBEEF;
            u32 separate2 = 0xCAFEBABE;
            u32 expected_separate1 = 0xDEADBEEF;
            u32 expected_separate2 = 0xCAFEBABE;
            u32 *expected_out1 = out1_alias == 0 ? &expected_separate1 :
                                 &expected_words[out1_alias - 1];
            u32 *expected_out2 = out2_alias == 0 ? &expected_separate2 :
                                 &expected_words[out2_alias - 1];
            u32 *out1 = out1_alias == 0 ? &separate1 : &words[out1_alias - 1];
            u32 *out2 = out2_alias == 0 ? &separate2 : &words[out2_alias - 1];

            /* Apply expected stores to an independent copy in target order. */
            *expected_out1 = expected_words[1];
            *expected_out2 = expected_words[0];

            CopyListOwnerWords(words, out1, out2);

            assert(memcmp(words, expected_words, sizeof(words)) == 0);
            assert(separate1 == expected_separate1);
            assert(separate2 == expected_separate2);
        }
    }

    /* Fixed nonalias oracle ensures both output destinations are exercised. */
    {
        u32 words[2] = {0x11223344, 0xA1B2C3D4};
        u32 out1 = 0xDEADBEEF;
        u32 out2 = 0xCAFEBABE;
        CopyListOwnerWords(words, &out1, &out2);
        assert(out1 == 0xA1B2C3D4);
        assert(out2 == 0x11223344);
        assert(words[0] == 0x11223344 && words[1] == 0xA1B2C3D4);
    }

    /* Both outputs can be the same external word; the second store wins. */
    {
        u32 words[2] = {0x01020304, 0xA0B0C0D0};
        u32 shared = 0;
        CopyListOwnerWords(words, &shared, &shared);
        assert(shared == 0x01020304);
        assert(words[0] == 0x01020304 && words[1] == 0xA0B0C0D0);
    }
}

static void TestListCopy(unsigned length)
{
    struct Container owner;
    struct Container owner_before;
    struct ListCountNode nodes[300];
    struct ListCountNode nodes_before[300];
    struct ListCountNode guard;
    struct ListCountNode *output[302];
    struct ListCountNode *output_before[302];
    unsigned i;

    assert(length <= 300);
    memset(&owner, 0xA5, sizeof(owner));
    memset(nodes, 0x5A, sizeof(nodes));
    for (i = 0; i < 300; i++)
        nodes[i].previous = 0;
    for (i = 0; i < length; i++) {
        nodes[i].previous = i ? &nodes[i - 1] : 0;
        nodes[i].next = i + 1 < length ? &nodes[i + 1] : 0;
    }
    owner.allocator_owner = (void *)0xD00DFEED;
    owner.list_head = length ? &nodes[0] : 0;
    for (i = 0; i < 302; i++)
        output[i] = &guard;

    owner_before = owner;
    memcpy(nodes_before, nodes, sizeof(nodes));
    memcpy(output_before, output, sizeof(output));

    assert(CopyListNodes(&owner, output + 1) == 0);
    for (i = 0; i < length; i++)
        assert(output[i + 1] == &nodes[i]);
    assert(output[0] == output_before[0]);
    for (i = length + 1; i < 302; i++)
        assert(output[i] == output_before[i]);
    assert(memcmp(&owner_before, &owner, sizeof(owner)) == 0);
    assert(memcmp(nodes_before, nodes, sizeof(nodes)) == 0);
}

int main(void)
{
    TestWord0Getter();
    TestWordCopyAliases();
    TestListCopy(0);
    TestListCopy(1);
    TestListCopy(255);
    TestListCopy(256);
    TestListCopy(300);
    return 0;
}
