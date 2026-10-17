#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "../../include/tree_count.h"
#include "../../src/nonmatching/tree_root_count.c"

#define MAX_NODES 700

static struct TreeCountNode nodes[MAX_NODES];
static struct TreeCountNode decoys[4];

static unsigned oracle_count(const struct TreeCountNode *node)
{
    if (node == 0)
        return 0;
    return 1 + oracle_count(node->child_a) + oracle_count(node->child_b);
}

static void reset_nodes(void)
{
    unsigned i;
    memset(nodes, 0x5A, sizeof(nodes));
    memset(decoys, 0xC3, sizeof(decoys));
    for (i = 0; i < MAX_NODES; i++) {
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

static unsigned oracle_from(const struct TreeCountNode *root)
{
    return oracle_count(root);
}

static void verify(const struct TreeCountNode *root, unsigned expected)
{
    struct TreeCountNode nodes_before[MAX_NODES];
    struct TreeCountNode decoys_before[4];
    memcpy(nodes_before, nodes, sizeof(nodes));
    memcpy(decoys_before, decoys, sizeof(decoys));
    assert(CountTreeRootNodes(root) == expected);
    assert(oracle_from(root) == expected);
    assert(memcmp(nodes, nodes_before, sizeof(nodes)) == 0);
    assert(memcmp(decoys, decoys_before, sizeof(decoys)) == 0);
}

int main(void)
{
    unsigned i;

    /* Native host pointers widen the fixture fields; the target offsets are
       parent/child_a/child_b = 8/12/16, host offsets are 8/16/24. */
    assert(offsetof(struct TreeCountNode, parent) ==
           (sizeof(void *) == 4 ? 8 : 16));
    assert(offsetof(struct TreeCountNode, child_a) ==
           (sizeof(void *) == 4 ? 12 : 24));
    assert(offsetof(struct TreeCountNode, child_b) ==
           (sizeof(void *) == 4 ? 16 : 32));

    reset_nodes();
    verify(0, 0);
    nodes[0].unknown00 = 0x12345678;
    nodes[0].unknown04 = 0x89ABCDEF;
    verify(&nodes[0], 1);

    reset_nodes();
    for (i = 0; i < 8; i++) link_a(i, i + 1);
    verify(&nodes[0], 9);
    reset_nodes();
    for (i = 0; i < 8; i++) link_b(i, i + 1);
    verify(&nodes[0], 9);

    reset_nodes();
    for (i = 0; i < 15; i++) {
        link_a(i, i * 2 + 1);
        link_b(i, i * 2 + 2);
    }
    verify(&nodes[0], 31);

    reset_nodes();
    link_b(0, 1); link_a(0, 2);
    link_a(1, 3); link_b(1, 4);
    link_b(2, 5); link_a(2, 6);
    link_a(3, 7); link_b(4, 8);
    link_b(5, 9); link_a(6, 10);
    link_a(8, 11); link_b(8, 12);
    link_b(10, 13); link_a(10, 14);
    link_a(12, 15); link_b(12, 16);
    verify(&nodes[0], 17);

    /* A 513-node tree exercises counts beyond one byte. */
    reset_nodes();
    for (i = 0; i < 256; i++) {
        link_a(i, 257 + i);
        link_b(i, i + 1);
    }
    verify(&nodes[0], 513);

    /* Root at node 5 has an external parent and sibling; neither may count. */
    reset_nodes();
    link_a(0, 1); link_b(0, 2);
    link_a(1, 3); link_b(1, 4);
    link_a(2, 5); link_b(2, 6);
    link_a(5, 7); link_b(5, 8);
    link_a(7, 9); link_b(8, 10);
    verify(&nodes[5], 5);
    verify(&nodes[1], 3);
    verify(&nodes[2], 7);
    return 0;
}
