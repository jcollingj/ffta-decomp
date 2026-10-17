#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "../../include/tree_count.h"
#include "../../src/nonmatching/tree_count.c"

#define MAX_NODES 700

static struct Container owner;
static struct TreeCountNode nodes[MAX_NODES];
static struct TreeCountNode decoys[4];

static unsigned oracle_count(const struct TreeCountNode *node)
{
    if (node == 0)
        return 0;
    return 1 + oracle_count(node->child_a) + oracle_count(node->child_b);
}

static void reset_nodes(unsigned n)
{
    unsigned i;
    memset(&owner, 0xA5, sizeof(owner));
    memset(nodes, 0x5A, sizeof(nodes));
    memset(decoys, 0xC3, sizeof(decoys));
    owner.allocator_owner = (void *)0x12345678;
    owner.tree_root = n ? &nodes[0] : 0;
    for (i = 0; i < n; i++) {
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

static void verify(unsigned expected)
{
    struct Container owner_before = owner;
    struct TreeCountNode nodes_before[MAX_NODES];
    struct TreeCountNode decoys_before[4];
    memcpy(nodes_before, nodes, sizeof(nodes));
    memcpy(decoys_before, decoys, sizeof(decoys));
    assert(CountTreeNodes(&owner) == expected);
    assert(oracle_count(owner.tree_root) == expected);
    assert(memcmp(&owner, &owner_before, sizeof(owner)) == 0);
    assert(memcmp(nodes, nodes_before, sizeof(nodes)) == 0);
    assert(memcmp(decoys, decoys_before, sizeof(decoys)) == 0);
}

int main(void)
{
    unsigned i;

    /* Native host pointers intentionally widen the fixture fields. Target
       node pointers are at offsets 8/12/16; host owner root is at 16. */
    assert(offsetof(struct TreeCountNode, parent) ==
           (sizeof(void *) == 4 ? 8 : 16));
    assert(offsetof(struct TreeCountNode, child_a) ==
           (sizeof(void *) == 4 ? 12 : 24));
    assert(offsetof(struct TreeCountNode, child_b) ==
           (sizeof(void *) == 4 ? 16 : 32));
    assert(offsetof(struct Container, tree_root) == 16);

    reset_nodes(0);
    verify(0);

    reset_nodes(1);
    verify(1);

    /* Left-only and right-only skewed trees distinguish traversal branches. */
    reset_nodes(9);
    for (i = 0; i < 8; i++) link_a(i, i + 1);
    verify(9);
    reset_nodes(9);
    for (i = 0; i < 8; i++) link_b(i, i + 1);
    verify(9);

    /* Full balanced tree and mixed tree with both child links represented. */
    reset_nodes(31);
    for (i = 0; i < 15; i++) {
        link_a(i, i * 2 + 1);
        link_b(i, i * 2 + 2);
    }
    verify(31);
    reset_nodes(17);
    link_b(0, 1); link_a(0, 2);
    link_a(1, 3); link_b(1, 4);
    link_b(2, 5); link_a(2, 6);
    link_a(3, 7); link_b(4, 8);
    link_b(5, 9); link_a(6, 10);
    link_a(8, 11); link_b(8, 12);
    link_b(10, 13); link_a(10, 14);
    link_a(12, 15); link_b(12, 16);
    verify(17);

    /* A count wider than a byte, with the rest of the pool unreachable. */
    reset_nodes(513);
    for (i = 0; i < 256; i++) {
        link_a(i, 257 + i);
        link_b(i, i + 1);
    }
    verify(513);
    return 0;
}
