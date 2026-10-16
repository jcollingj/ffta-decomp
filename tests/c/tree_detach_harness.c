#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "../../include/tree_detach.h"

#define N 10
#define NONE N
static struct TreeCountNode nodes[N];
static struct TreeCountNode expected_nodes[N];
static struct Container owner;
static struct Container expected_owner;

static struct TreeCountNode *ptr(unsigned i)
{
    return i == NONE ? 0 : &nodes[i];
}

static void node_links(struct TreeCountNode *array, unsigned node,
                       unsigned parent, unsigned a, unsigned b)
{
    array[node].parent = ptr(parent);
    array[node].child_a = ptr(a);
    array[node].child_b = ptr(b);
}

static void reset(void)
{
    unsigned i;
    memset(nodes, 0, sizeof nodes);
    memset(expected_nodes, 0, sizeof expected_nodes);
    memset(&owner, 0, sizeof owner);
    memset(&expected_owner, 0, sizeof expected_owner);
    owner.allocator_owner = (void *)(uintptr_t)0x12345678u;
    owner.list_head = (struct ListCountNode *)(uintptr_t)0x87654321u;
    for (i = 0; i < N; ++i) {
        nodes[i].unknown00 = 50;
        nodes[i].unknown04 = 0xA5000000u + i;
        nodes[i].flags = 0x12340000u + i;
    }
}

static void expected_begin(void)
{
    memcpy(expected_nodes, nodes, sizeof nodes);
    expected_owner = owner;
}

static void finish(unsigned removed)
{
    DetachTreeNode(&owner, &nodes[removed]);
    assert(memcmp(&owner, &expected_owner, sizeof owner) == 0);
    assert(memcmp(nodes, expected_nodes, sizeof nodes) == 0);
}

static void root_sole(void)
{
    reset();
    owner.tree_root = &nodes[0];
    expected_begin();
    expected_owner.tree_root = 0;
    finish(0);
}

static void root_one_child(unsigned child, unsigned is_a)
{
    reset();
    nodes[0].unknown00 = 50;
    nodes[child].unknown00 = is_a ? 20 : 80;
    owner.tree_root = &nodes[0];
    if (is_a)
        node_links(nodes, 0, NONE, child, NONE);
    else
        node_links(nodes, 0, NONE, NONE, child);
    node_links(nodes, child, 0, NONE, NONE);
    expected_begin();
    expected_owner.tree_root = &nodes[child];
    node_links(expected_nodes, child, NONE, NONE, NONE);
    node_links(expected_nodes, 0, NONE, NONE, NONE);
    finish(0);
}

static void root_two_children(void)
{
    reset();
    nodes[0].unknown00 = 50; nodes[1].unknown00 = 20; nodes[2].unknown00 = 80;
    owner.tree_root = &nodes[0];
    node_links(nodes, 0, NONE, 1, 2);
    node_links(nodes, 1, 0, NONE, NONE);
    node_links(nodes, 2, 0, NONE, NONE);
    expected_begin();
    expected_owner.tree_root = &nodes[1];
    node_links(expected_nodes, 0, NONE, NONE, NONE);
    node_links(expected_nodes, 1, NONE, NONE, 2);
    node_links(expected_nodes, 2, 1, NONE, NONE);
    finish(0);
}

static void nonroot_leaf(unsigned removed)
{
    reset();
    nodes[0].unknown00 = 50; nodes[1].unknown00 = 20; nodes[2].unknown00 = 80;
    owner.tree_root = &nodes[0];
    node_links(nodes, 0, NONE, 1, 2);
    node_links(nodes, 1, 0, NONE, NONE);
    node_links(nodes, 2, 0, NONE, NONE);
    expected_begin();
    if (removed == 1) {
        node_links(expected_nodes, 0, NONE, NONE, 2);
        node_links(expected_nodes, 1, NONE, NONE, NONE);
    } else {
        node_links(expected_nodes, 0, NONE, 1, NONE);
        node_links(expected_nodes, 2, NONE, NONE, NONE);
    }
    finish(removed);
}

static void nonroot_two_children(void)
{
    reset();
    nodes[0].unknown00 = 50; nodes[1].unknown00 = 20; nodes[2].unknown00 = 80;
    nodes[3].unknown00 = 10; nodes[4].unknown00 = 30;
    nodes[5].unknown00 = 70; nodes[6].unknown00 = 90;
    owner.tree_root = &nodes[0];
    node_links(nodes, 0, NONE, 1, 2);
    node_links(nodes, 1, 0, 3, 4);
    node_links(nodes, 2, 0, 5, 6);
    node_links(nodes, 3, 1, NONE, NONE); node_links(nodes, 4, 1, NONE, NONE);
    node_links(nodes, 5, 2, NONE, NONE); node_links(nodes, 6, 2, NONE, NONE);
    expected_begin();
    node_links(expected_nodes, 0, NONE, 3, 2);
    node_links(expected_nodes, 1, NONE, NONE, NONE);
    node_links(expected_nodes, 3, 0, NONE, 4);
    node_links(expected_nodes, 4, 3, NONE, NONE);
    finish(1);
}

static void deep_reinsert(void)
{
    reset();
    nodes[0].unknown00 = 50; nodes[1].unknown00 = 10; nodes[2].unknown00 = 80;
    nodes[3].unknown00 = 20; nodes[4].unknown00 = 30;
    owner.tree_root = &nodes[0];
    node_links(nodes, 0, NONE, 1, 2); node_links(nodes, 1, 0, NONE, 3);
    node_links(nodes, 2, 0, NONE, NONE); node_links(nodes, 3, 1, NONE, 4);
    node_links(nodes, 4, 3, NONE, NONE);
    expected_begin();
    expected_owner.tree_root = &nodes[1];
    node_links(expected_nodes, 0, NONE, NONE, NONE);
    node_links(expected_nodes, 1, NONE, NONE, 3);
    node_links(expected_nodes, 2, 4, NONE, NONE);
    node_links(expected_nodes, 3, 1, NONE, 4);
    node_links(expected_nodes, 4, 3, NONE, 2);
    finish(0);
}

static void equal_and_signed_boundaries(void)
{
    /* Equal keys on both sides violate the duplicate-goes-right search-tree
       invariant. This deliberately probes the original >= right branch. */
    reset();
    nodes[0].unknown00 = 7; nodes[1].unknown00 = 7; nodes[2].unknown00 = 7;
    owner.tree_root = &nodes[0];
    node_links(nodes, 0, NONE, 1, 2); node_links(nodes, 1, 0, NONE, NONE);
    node_links(nodes, 2, 0, NONE, NONE);
    expected_begin();
    expected_owner.tree_root = &nodes[1];
    node_links(expected_nodes, 0, NONE, NONE, NONE);
    node_links(expected_nodes, 1, NONE, NONE, 2);
    node_links(expected_nodes, 2, 1, NONE, NONE);
    finish(0);

    /* The original tree is valid for signed ordering: MIN < -1 < 0, with
       removed root -1, child A MIN, and child B 0. Reinsertion of 0 under
       promoted MIN must follow child B under signed comparison. */
    reset();
    nodes[0].unknown00 = 0xffffffffu;
    nodes[1].unknown00 = 0x80000000u;
    nodes[2].unknown00 = 0;
    owner.tree_root = &nodes[0];
    node_links(nodes, 0, NONE, 1, 2); node_links(nodes, 1, 0, NONE, NONE);
    node_links(nodes, 2, 0, NONE, NONE);
    expected_begin();
    expected_owner.tree_root = &nodes[1];
    node_links(expected_nodes, 0, NONE, NONE, NONE);
    node_links(expected_nodes, 1, NONE, NONE, 2);
    node_links(expected_nodes, 2, 1, NONE, NONE);
    finish(0);
}

int main(void)
{
    root_sole();
    root_one_child(1, 1);
    root_one_child(1, 0);
    root_two_children();
    nonroot_leaf(1);
    nonroot_leaf(2);
    nonroot_two_children();
    deep_reinsert();
    equal_and_signed_boundaries();
    return 0;
}
