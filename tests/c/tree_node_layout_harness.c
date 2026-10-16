#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "../../include/list_callbacks.h"
#include "../../include/tree_count.h"

#define NODE_COUNT 3

static struct Container owner;
static struct TreeCountNode nodes[NODE_COUNT];

static void setup(void)
{
    memset(&owner, 0xA5, sizeof(owner));
    memset(nodes, 0x5A, sizeof(nodes));
    owner.tree_root = &nodes[1];

    nodes[0].unknown00 = 0x10;
    nodes[0].unknown04 = 0x1010;
    nodes[0].parent = &nodes[1];
    nodes[0].child_a = 0;
    nodes[0].child_b = 0;
    nodes[0].flags = 0xA5000000u;

    nodes[1].unknown00 = 0x20;
    nodes[1].unknown04 = 0x2020;
    nodes[1].parent = 0;
    nodes[1].child_a = &nodes[0];
    nodes[1].child_b = &nodes[2];
    nodes[1].flags = 0xA5000000u;

    nodes[2].unknown00 = 0x30;
    nodes[2].unknown04 = 0x3030;
    nodes[2].parent = &nodes[1];
    nodes[2].child_a = 0;
    nodes[2].child_b = 0;
    nodes[2].flags = 0xA5000001u;
}

static void test_select_callback_composition(void)
{
    struct Container owner_before;
    struct TreeCountNode nodes_before[NODE_COUNT];
    static const u32 flags[4][NODE_COUNT] = {
        { 0xA5000000u, 0xA5000001u, 0xA5000001u },
        { 0xA5000001u, 0xA5000000u, 0xA5000001u },
        { 0xA5000001u, 0xA5000001u, 0xA5000000u },
        { 0xA5000001u, 0xA5000001u, 0xA5000001u }
    };
    unsigned scenario;

    for (scenario = 0; scenario < 4; scenario++) {
        void *selected = &owner;
        unsigned i;

        setup();
        for (i = 0; i < NODE_COUNT; i++)
            nodes[i].flags = flags[scenario][i];
        owner_before = owner;
        memcpy(nodes_before, nodes, sizeof(nodes));

        TraverseTreeInOrder(&owner, ListCallbackSelectClear, &selected);

        if (scenario == 0)
            assert(selected == &nodes[0]);
        else if (scenario == 1)
            assert(selected == &nodes[1]);
        else if (scenario == 2)
            assert(selected == &nodes[2]);
        else
            assert(selected == &owner);
        assert(memcmp(&owner, &owner_before, sizeof(owner)) == 0);
        assert(memcmp(nodes, nodes_before, sizeof(nodes)) == 0);
    }
}

static void test_clear_callback_composition(void)
{
    struct Container owner_before;
    struct TreeCountNode nodes_before[NODE_COUNT];
    struct TreeCountNode expected[NODE_COUNT];

    setup();
    owner_before = owner;
    memcpy(nodes_before, nodes, sizeof(nodes));
    memcpy(expected, nodes, sizeof(nodes));
    expected[0].flags = 0xA5000000u;
    expected[1].flags = 0xA5000000u;
    expected[2].flags = 0xA5000000u;

    TraverseTreeInOrder(&owner, ListCallbackClearFlag0, 0);

    assert(memcmp(&owner, &owner_before, sizeof(owner)) == 0);
    assert(memcmp(nodes, expected, sizeof(nodes)) == 0);
    assert(nodes_before[2].flags != nodes[2].flags);
}

int main(void)
{
    assert(offsetof(struct TreeCountNode, parent) ==
           (sizeof(void *) == 4 ? 8 : 8));
    assert(offsetof(struct TreeCountNode, flags) ==
           (sizeof(void *) == 4 ? 20 : 32));
    assert(sizeof(struct TreeCountNode) ==
           (sizeof(void *) == 4 ? 24 : 40));
    test_select_callback_composition();
    test_clear_callback_composition();
    return 0;
}
