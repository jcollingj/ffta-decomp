#include <assert.h>
#include <string.h>
#include "../../include/tree_collect.h"
#include "../../src/tree_collect.c"
#include "../../src/list_traversal.c"
#include "../../src/tree_reverse_visit.c"
#include "../../src/list_callbacks.c"

#define NODE_COUNT 7
#define OUTPUT_COUNT 9

static struct Container owner;
static struct TreeCountNode nodes[NODE_COUNT];
static int sentinel_values[2][OUTPUT_COUNT];

static void reset_tree(void)
{
    unsigned i;

    memset(&owner, 0xA7, sizeof(owner));
    memset(nodes, 0xD1, sizeof(nodes));
    for (i = 0; i < NODE_COUNT; i++) {
        nodes[i].unknown00 = i;
        nodes[i].parent = 0;
        nodes[i].child_a = 0;
        nodes[i].child_b = 0;
    }
}

static void link_a(unsigned parent, unsigned child)
{
    nodes[parent].child_a = &nodes[child];
    nodes[child].parent = &nodes[parent];
}

static void link_b(unsigned parent, unsigned child)
{
    nodes[parent].child_b = &nodes[child];
    nodes[child].parent = &nodes[parent];
}

static void make_tree(void)
{
    reset_tree();
    owner.tree_root = &nodes[0];
    link_a(0, 1);
    link_b(0, 2);
    link_a(1, 3);
    link_b(1, 4);
    link_a(2, 5);
    link_b(2, 6);
}

static void fill_sentinels(void **output, unsigned lane)
{
    unsigned i;
    for (i = 0; i < OUTPUT_COUNT; i++)
        output[i] = &sentinel_values[lane][i];
}

static void test_empty_root_guards(void)
{
    struct Container owner_before;
    struct TreeCountNode nodes_before[NODE_COUNT];
    void *forward[OUTPUT_COUNT];
    void *reverse[OUTPUT_COUNT];
    void *forward_before[OUTPUT_COUNT];
    void *reverse_before[OUTPUT_COUNT];

    reset_tree();
    owner.tree_root = 0;
    owner_before = owner;
    memcpy(nodes_before, nodes, sizeof(nodes));
    fill_sentinels(forward, 0);
    fill_sentinels(reverse, 1);
    memcpy(forward_before, forward, sizeof(forward));
    memcpy(reverse_before, reverse, sizeof(reverse));

    assert(TreeCollectInOrder(&owner, forward) == 0);
    assert(TreeCollectReverseInOrder(&owner, reverse) == 0);
    assert(memcmp(&owner, &owner_before, sizeof(owner)) == 0);
    assert(memcmp(nodes, nodes_before, sizeof(nodes)) == 0);
    assert(memcmp(forward, forward_before, sizeof(forward)) == 0);
    assert(memcmp(reverse, reverse_before, sizeof(reverse)) == 0);
}

static void test_real_forward_and_reverse_collectors(void)
{
    static const unsigned forward_ids[] = { 3, 1, 4, 0, 5, 2, 6 };
    static const unsigned reverse_ids[] = { 6, 2, 5, 0, 4, 1, 3 };
    struct Container owner_before;
    struct TreeCountNode nodes_before[NODE_COUNT];
    void *forward[OUTPUT_COUNT];
    void *reverse[OUTPUT_COUNT];
    void *forward_before[OUTPUT_COUNT];
    void *reverse_before[OUTPUT_COUNT];
    unsigned i;

    make_tree();
    owner_before = owner;
    memcpy(nodes_before, nodes, sizeof(nodes));
    fill_sentinels(forward, 0);
    fill_sentinels(reverse, 1);
    memcpy(forward_before, forward, sizeof(forward));
    memcpy(reverse_before, reverse, sizeof(reverse));

    assert(TreeCollectInOrder(&owner, forward) ==
           sizeof(forward_ids) / sizeof(forward_ids[0]));
    for (i = 0; i < sizeof(forward_ids) / sizeof(forward_ids[0]); i++)
        assert(forward[i] == &nodes[forward_ids[i]]);
    for (i = sizeof(forward_ids) / sizeof(forward_ids[0]); i < OUTPUT_COUNT; i++)
        assert(forward[i] == forward_before[i]);

    assert(TreeCollectReverseInOrder(&owner, reverse) ==
           sizeof(reverse_ids) / sizeof(reverse_ids[0]));
    for (i = 0; i < sizeof(reverse_ids) / sizeof(reverse_ids[0]); i++)
        assert(reverse[i] == &nodes[reverse_ids[i]]);
    for (i = sizeof(reverse_ids) / sizeof(reverse_ids[0]); i < OUTPUT_COUNT; i++)
        assert(reverse[i] == reverse_before[i]);

    assert(memcmp(&owner, &owner_before, sizeof(owner)) == 0);
    assert(memcmp(nodes, nodes_before, sizeof(nodes)) == 0);
}

int main(void)
{
    test_empty_root_guards();
    test_real_forward_and_reverse_collectors();
    return 0;
}
