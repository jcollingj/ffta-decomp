#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "../../include/list_accessors.h"

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

static void TestWordCopyTypedAliases(void)
{
    {
        struct TreeCountNode node;
        ListPayload out_word4 = 0xDEADBEEFUL;
        u32 out_word0 = 0xCAFEBABEu;
        struct TreeCountNode before;

        memset(&node, 0x5A, sizeof(node));
        node.unknown00 = 0x11223344u;
        node.unknown04 = 0xA1B2C3D4UL;
        before = node;
        CopyListOwnerWords(&node, &out_word4, &out_word0);
        assert(out_word4 == 0xA1B2C3D4UL);
        assert(out_word0 == 0x11223344u);
        assert(node.unknown00 == 0x11223344u);
        assert(node.unknown04 == 0xA1B2C3D4UL);
        assert(memcmp(&before, &node, sizeof(node)) == 0);
    }

    {
        struct TreeCountNode node;
        struct TreeCountNode before;

        memset(&node, 0x5A, sizeof(node));
        node.unknown00 = 0x11223344u;
        node.unknown04 = 0xA1B2C3D4UL;
        before = node;
        CopyListOwnerWords(&node, &node.unknown04, &node.unknown00);
        assert(node.unknown00 == 0x11223344u);
        assert(node.unknown04 == 0xA1B2C3D4UL);
        assert(memcmp(&before, &node, sizeof(node)) == 0);
    }

    /* The output types have different widths on LP64. A union gives the two
       typed destinations a shared address without casting either pointer. */
    {
        union {
            ListPayload payload;
            u32 key;
        } shared;
        struct TreeCountNode node;
        struct TreeCountNode before;

        memset(&node, 0x5A, sizeof(node));
        node.unknown00 = 0xA1B2C3D4u;
        node.unknown04 = (ListPayload)0x1122334455667788ULL;
        before = node;
        shared.payload = (ListPayload)0xFFEEDDCCBBAA0099ULL;
        CopyListOwnerWords(&node, &shared.payload, &shared.key);
        if (sizeof(ListPayload) == 8)
            assert(shared.payload == (ListPayload)0x11223344A1B2C3D4ULL);
        else
            assert(shared.key == 0xA1B2C3D4u);
        assert(memcmp(&before, &node, sizeof(node)) == 0);
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
    TestWordCopyTypedAliases();
    TestListCopy(0);
    TestListCopy(1);
    TestListCopy(255);
    TestListCopy(256);
    TestListCopy(300);
    return 0;
}
