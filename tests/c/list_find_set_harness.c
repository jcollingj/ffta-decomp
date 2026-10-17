#include <assert.h>
#include <string.h>
#include "../../include/list_count.h"
#include "../../include/list_find_word.h"
#include "../../include/list_set_words.h"

static void CheckSearch(const struct Container *owner, ListPayload word,
                        struct ListCountNode *expected,
                        struct Container *owner_storage,
                        struct ListCountNode *nodes, unsigned node_count)
{
    struct Container owner_before = *owner_storage;
    struct ListCountNode nodes_before[520];

    assert(node_count <= 520);
    memcpy(nodes_before, nodes, node_count * sizeof(*nodes));
    assert(FindListNodeByWord(owner, word) == expected);
    assert(memcmp(&owner_before, owner_storage, sizeof(owner_before)) == 0);
    assert(memcmp(nodes_before, nodes, node_count * sizeof(*nodes)) == 0);
}

static void TestFind(void)
{
    struct Container owner;
    struct ListCountNode nodes[520];
    unsigned i;

    memset(&owner, 0xA5, sizeof(owner));
    memset(nodes, 0x5A, sizeof(nodes));
    for (i = 0; i < 520; i++)
        nodes[i].previous = 0;
    owner.allocator_owner = (void *)0xD00DFEED;
    owner.list_head = 0;
    CheckSearch(&owner, 0, 0, &owner, nodes, 520);

    owner.list_head = &nodes[0];
    nodes[0].unknown00 = 0;
    nodes[0].previous = 0;
    nodes[0].next = &nodes[1];
    nodes[1].unknown00 = 0;
    nodes[1].previous = &nodes[0];
    nodes[1].next = 0;
    CheckSearch(&owner, 0, &nodes[0], &owner, nodes, 520);

    for (i = 0; i < 519; i++) {
        nodes[i].unknown00 = 0x10000000u + i;
        nodes[i].previous = i ? &nodes[i - 1] : 0;
        nodes[i].next = &nodes[i + 1];
    }
    nodes[519].unknown00 = 0xF1234567u;
    nodes[519].previous = &nodes[518];
    nodes[519].next = 0;
    owner.list_head = &nodes[0];
    CheckSearch(&owner, 0xF1234567u, &nodes[519], &owner, nodes, 520);

    nodes[17].unknown00 = 0x76543210u;
    nodes[400].unknown00 = 0x76543210u;
    CheckSearch(&owner, 0x76543210u, &nodes[17], &owner, nodes, 520);
    CheckSearch(&owner, 0xDEADBEEFu, 0, &owner, nodes, 520);
}

static void TestSetWords(void)
{
    struct TreeCountNode node;
    struct TreeCountNode expected;
    static int retained_payload;
    ListPayload pointer_payload = (ListPayload)(unsigned long)&retained_payload;

    memset(&node, 0x5A, sizeof(node));
    expected = node;
    node.unknown00 = 0x11223344u;
    node.unknown04 = (ListPayload)0xAABBCCDDEEFF0011ULL;
    expected.unknown00 = 0x76543210u;
    expected.unknown04 = (ListPayload)0xAABBCCDDEEFF0011ULL;
    SetListWords(&node, (ListPayload)0xAABBCCDDEEFF0011ULL, 0x76543210u);
    assert(memcmp(&node, &expected, sizeof(node)) == 0);

    expected = node;
    expected.unknown00 = 0x89ABCDEFu;
    expected.unknown04 = pointer_payload;
    SetListWords(&node, pointer_payload, 0x89ABCDEFu);
    assert(memcmp(&node, &expected, sizeof(node)) == 0);
    assert(node.unknown04 == pointer_payload);
    assert((void *)(unsigned long)node.unknown04 == &retained_payload);
}

int main(void)
{
    TestFind();
    TestSetWords();
    return 0;
}
