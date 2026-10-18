#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "list_callbacks.h"
#include "tree_detach.h"
#include "tree_rebuild.h"
#include "tree_reverse_visit.h"

#define NODE_COUNT 5
#define NONE NODE_COUNT

static struct TreeCountNode nodes[NODE_COUNT];
static struct TreeCountNode expected_nodes[NODE_COUNT];
static struct Container owner;
static struct Container expected_owner;
#ifndef TREE_REBUILD_COMPOSITION_TEST
static unsigned detached[NODE_COUNT];
static unsigned detach_count;
#endif

static struct TreeCountNode *node_ptr(unsigned index)
{
    return index == NONE ? 0 : &nodes[index];
}

static void set_links(struct TreeCountNode *array, unsigned index,
                      unsigned parent, unsigned child_a, unsigned child_b)
{
    array[index].parent = node_ptr(parent);
    array[index].child_a = node_ptr(child_a);
    array[index].child_b = node_ptr(child_b);
}

static void initialize_actual(void)
{
    unsigned i;

    memset(nodes, 0, sizeof nodes);
    memset(&owner, 0, sizeof owner);
    owner.allocator_owner = (void *)(uintptr_t)0x12345678u;
    owner.list_head = (struct ListCountNode *)(uintptr_t)0x87654321u;
    for (i = 0; i < NODE_COUNT; ++i) {
        nodes[i].unknown00 = 0x11000000u + i;
        nodes[i].unknown04 = (ListPayload)0xA5000000u + i;
        nodes[i].flags = 0x22000000u + i;
    }
#ifndef TREE_REBUILD_COMPOSITION_TEST
    detach_count = 0;
#endif
}

static void initialize_expected(void)
{
    unsigned i;

    memset(expected_nodes, 0, sizeof expected_nodes);
    memset(&expected_owner, 0, sizeof expected_owner);
    expected_owner.allocator_owner = (void *)(uintptr_t)0x12345678u;
    expected_owner.list_head = (struct ListCountNode *)(uintptr_t)0x87654321u;
    for (i = 0; i < NODE_COUNT; ++i) {
        expected_nodes[i].unknown00 = 0x11000000u + i;
        expected_nodes[i].unknown04 = (ListPayload)0xA5000000u + i;
        expected_nodes[i].flags = 0x22000000u + i;
    }
}

static void assert_whole_state(void)
{
    assert(memcmp(&owner, &expected_owner, sizeof owner) == 0);
    assert(memcmp(nodes, expected_nodes, sizeof nodes) == 0);
}

#ifndef TREE_REBUILD_COMPOSITION_TEST
/* Controlled test dependency: only unlink a selected leaf. This deliberately
   does not implement the candidate detach/reinsert behavior. */
void DetachTreeNode(struct Container *actual_owner,
                    struct TreeCountNode *node)
{
    unsigned i;
    unsigned index = NODE_COUNT;

    assert(actual_owner == &owner);
    assert(node->child_a == 0 && node->child_b == 0);
    for (i = 0; i < NODE_COUNT; ++i) {
        if (node == &nodes[i])
            index = i;
    }
    assert(index < NODE_COUNT);
    assert(detach_count < NODE_COUNT);
    detached[detach_count++] = index;

    if (node->parent == 0) {
        assert(owner.tree_root == node);
        owner.tree_root = 0;
    } else if (node->parent->child_a == node) {
        node->parent->child_a = 0;
    } else {
        assert(node->parent->child_b == node);
        node->parent->child_b = 0;
    }
    node->parent = 0;
}
#endif

static void empty_tree(void)
{
    initialize_actual();
    initialize_expected();
    assert_whole_state();
    RebuildTreeByNumericKey(&owner);
    assert_whole_state();
#ifndef TREE_REBUILD_COMPOSITION_TEST
    assert(detach_count == 0);
#endif
}

static void single_node(void)
{
    initialize_actual();
    initialize_expected();
    nodes[0].unknown00 = 19;
    nodes[0].flags = 0x80000000u;
    owner.tree_root = &nodes[0];
    expected_owner.tree_root = &nodes[0];
    expected_nodes[0].unknown00 = 19;
    expected_nodes[0].flags = 0x80000000u;

    RebuildTreeByNumericKey(&owner);
    assert_whole_state();
#ifndef TREE_REBUILD_COMPOSITION_TEST
    assert(detach_count == 1);
    assert(detached[0] == 0);
#endif
}

static void signed_extrema(void)
{
    initialize_actual();
    initialize_expected();
    nodes[0].unknown00 = 0;
    nodes[1].unknown00 = 0x80000000u;
    nodes[2].unknown00 = 0x7fffffffu;
    nodes[0].flags = 0x80000001u;
    nodes[1].flags = 0x80000020u;
    nodes[2].flags = 0x20u;
    owner.tree_root = &nodes[0];
    set_links(nodes, 0, NONE, 1, 2);
    set_links(nodes, 1, 0, NONE, NONE);
    set_links(nodes, 2, 0, NONE, NONE);

    expected_owner.tree_root = &nodes[0];
    expected_nodes[0].unknown00 = 0;
    expected_nodes[0].flags = 0x80000000u;
    expected_nodes[1].unknown00 = 0x80000000u;
    expected_nodes[1].flags = 0x80000020u;
    expected_nodes[2].unknown00 = 0x7fffffffu;
    expected_nodes[2].flags = 0x20u;
    set_links(expected_nodes, 0, NONE, 1, 2);
    set_links(expected_nodes, 1, 0, NONE, NONE);
    set_links(expected_nodes, 2, 0, NONE, NONE);

    RebuildTreeByNumericKey(&owner);
    assert_whole_state();
#ifndef TREE_REBUILD_COMPOSITION_TEST
    assert(detach_count == 2);
    assert(detached[0] == 2 && detached[1] == 1);
#endif
}

static void equal_key_routes_right(void)
{
    initialize_actual();
    initialize_expected();
    nodes[0].unknown00 = 7;
    nodes[1].unknown00 = 7;
    nodes[0].flags = 0x81u;
    nodes[1].flags = 0x80000020u;
    owner.tree_root = &nodes[0];
    set_links(nodes, 0, NONE, NONE, 1);
    set_links(nodes, 1, 0, NONE, NONE);

    expected_owner.tree_root = &nodes[0];
    expected_nodes[0].unknown00 = 7;
    expected_nodes[0].flags = 0x80u;
    expected_nodes[1].unknown00 = 7;
    expected_nodes[1].flags = 0x80000020u;
    set_links(expected_nodes, 0, NONE, NONE, 1);
    set_links(expected_nodes, 1, 0, NONE, NONE);

    RebuildTreeByNumericKey(&owner);
    assert_whole_state();
#ifndef TREE_REBUILD_COMPOSITION_TEST
    assert(detach_count == 1 && detached[0] == 1);
#endif
}

#ifdef TREE_REBUILD_COMPOSITION_TEST
static unsigned visit_order[NODE_COUNT];
static unsigned visit_count;

static int record_visit(void *value, void *context)
{
    unsigned i;

    (void)context;
    for (i = 0; i < NODE_COUNT; ++i) {
        if (value == &nodes[i])
            break;
    }
    assert(i < NODE_COUNT);
    visit_order[visit_count++] = i;
    return 1;
}

static void unsorted_multi_node_composition(void)
{
    /* R(50).child_b=C(10).child_b=E(70) is a valid linked tree whose keys
       violate the current numeric ordering. Rebuild selects E, then C. */
    initialize_actual();
    initialize_expected();
    nodes[0].unknown00 = 50;
    nodes[1].unknown00 = 10;
    nodes[2].unknown00 = 70;
    nodes[0].flags = 0x81u;
    nodes[1].flags = 0x80000000u;
    nodes[2].flags = 0x20u;
    owner.tree_root = &nodes[0];
    set_links(nodes, 0, NONE, NONE, 1);
    set_links(nodes, 1, 0, NONE, 2);
    set_links(nodes, 2, 1, NONE, NONE);

    /* The real reverse traversal's complete visit order is E,C,R. */
    visit_count = 0;
    TraverseTreeReverseInOrder(&owner, record_visit, 0);
    assert(visit_count == 3);
    assert(visit_order[0] == 2 && visit_order[1] == 1 && visit_order[2] == 0);

    /* Hand-derived final links: E is inserted by a two-edge search, then
       detaching C moves E to R.child_b before C enters R.child_a. */
    expected_owner.tree_root = &nodes[0];
    expected_nodes[0].unknown00 = 50;
    expected_nodes[0].flags = 0x80u;
    expected_nodes[1].unknown00 = 10;
    expected_nodes[1].flags = 0x80000000u;
    expected_nodes[2].unknown00 = 70;
    expected_nodes[2].flags = 0x20u;
    set_links(expected_nodes, 0, NONE, 1, 2);
    set_links(expected_nodes, 1, 0, NONE, NONE);
    set_links(expected_nodes, 2, 0, NONE, NONE);

    RebuildTreeByNumericKey(&owner);
    assert_whole_state();
}
#endif

int main(void)
{
    empty_tree();
    single_node();
    signed_extrema();
    equal_key_routes_right();
#ifdef TREE_REBUILD_COMPOSITION_TEST
    unsorted_multi_node_composition();
#endif
    return 0;
}
