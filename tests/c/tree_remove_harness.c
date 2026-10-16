#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "../../include/allocator_wrappers.h"
#include "../../include/tree_remove.h"

#define NODE_COUNT 3

static struct Container owner;
static struct Container expected_before_owner;
static struct Container expected_after_owner;
static struct TreeCountNode nodes[NODE_COUNT];
static struct TreeCountNode expected_before_nodes[NODE_COUNT];
static struct TreeCountNode expected_after_nodes[NODE_COUNT];
static unsigned trace[2];
static unsigned trace_count;
static void *initial_allocator;
static void *replacement_allocator;

enum { TRACE_DETACH = 1, TRACE_FREE = 2 };

void DetachTreeNode(struct Container *actual_owner,
                    struct TreeCountNode *actual_node)
{
    assert(trace_count == 0);
    trace[trace_count++] = TRACE_DETACH;
    assert(actual_owner == &owner);
    assert(actual_node == &nodes[1]);
    assert(memcmp(actual_owner, &expected_before_owner, sizeof owner) == 0);
    assert(memcmp(nodes, expected_before_nodes, sizeof nodes) == 0);

    /* Model observable callback writes, including allocator replacement. */
    actual_owner->allocator_owner = replacement_allocator;
    actual_owner->tree_root = &nodes[2];
    actual_owner->list_head = (struct ListCountNode *)(uintptr_t)0x55556666u;
    nodes[0].unknown00 = 0xD0000000u;
    nodes[0].unknown04 = 0xD0000001u;
    nodes[0].parent = &nodes[2];
    nodes[0].child_a = &nodes[1];
    nodes[0].child_b = 0;
    nodes[0].flags = 0xD0000002u;
    nodes[1].unknown00 = 0xD1000000u;
    nodes[1].unknown04 = 0xD1000001u;
    nodes[1].parent = 0;
    nodes[1].child_a = &nodes[0];
    nodes[1].child_b = &nodes[2];
    nodes[1].flags = 0xD1000002u;
    nodes[2].unknown00 = 0xD2000000u;
    nodes[2].unknown04 = 0xD2000001u;
    nodes[2].parent = &nodes[1];
    nodes[2].child_a = 0;
    nodes[2].child_b = &nodes[0];
    nodes[2].flags = 0xD2000002u;
}

void AllocatorFree(void *allocator, void *allocation)
{
    assert(trace_count == 1);
    trace[trace_count++] = TRACE_FREE;
    assert(allocator == replacement_allocator);
    assert(allocator != initial_allocator);
    assert(allocation == &nodes[1]);
    assert(memcmp(&owner, &expected_after_owner, sizeof owner) == 0);
    assert(memcmp(nodes, expected_after_nodes, sizeof nodes) == 0);
    /* Deliberately retain storage so the caller can inspect every byte. */
}

static void run_wrapper_mutation_case(void)
{
    memset(&owner, 0, sizeof owner);
    memset(nodes, 0, sizeof nodes);
    memset(&expected_before_owner, 0, sizeof expected_before_owner);
    memset(&expected_after_owner, 0, sizeof expected_after_owner);
    memset(expected_before_nodes, 0, sizeof expected_before_nodes);
    memset(expected_after_nodes, 0, sizeof expected_after_nodes);

    initial_allocator = (void *)(uintptr_t)0x11112222u;
    replacement_allocator = (void *)(uintptr_t)0x33334444u;
    owner.allocator_owner = initial_allocator;
    owner.list_head = (struct ListCountNode *)(uintptr_t)0x77778888u;
    owner.tree_root = &nodes[0];
    nodes[0].unknown00 = 0xA0000000u;
    nodes[0].unknown04 = 0xA0000001u;
    nodes[0].parent = 0;
    nodes[0].child_a = &nodes[1];
    nodes[0].child_b = &nodes[2];
    nodes[0].flags = 0xA0000002u;
    nodes[1].unknown00 = 0xA1000000u;
    nodes[1].unknown04 = 0xA1000001u;
    nodes[1].parent = &nodes[0];
    nodes[1].child_a = 0;
    nodes[1].child_b = 0;
    nodes[1].flags = 0xA1000002u;
    nodes[2].unknown00 = 0xA2000000u;
    nodes[2].unknown04 = 0xA2000001u;
    nodes[2].parent = &nodes[0];
    nodes[2].child_a = 0;
    nodes[2].child_b = 0;
    nodes[2].flags = 0xA2000002u;

    expected_before_owner.allocator_owner = initial_allocator;
    expected_before_owner.list_head =
        (struct ListCountNode *)(uintptr_t)0x77778888u;
    expected_before_owner.tree_root = &nodes[0];
    expected_before_nodes[0].unknown00 = 0xA0000000u;
    expected_before_nodes[0].unknown04 = 0xA0000001u;
    expected_before_nodes[0].parent = 0;
    expected_before_nodes[0].child_a = &nodes[1];
    expected_before_nodes[0].child_b = &nodes[2];
    expected_before_nodes[0].flags = 0xA0000002u;
    expected_before_nodes[1].unknown00 = 0xA1000000u;
    expected_before_nodes[1].unknown04 = 0xA1000001u;
    expected_before_nodes[1].parent = &nodes[0];
    expected_before_nodes[1].child_a = 0;
    expected_before_nodes[1].child_b = 0;
    expected_before_nodes[1].flags = 0xA1000002u;
    expected_before_nodes[2].unknown00 = 0xA2000000u;
    expected_before_nodes[2].unknown04 = 0xA2000001u;
    expected_before_nodes[2].parent = &nodes[0];
    expected_before_nodes[2].child_a = 0;
    expected_before_nodes[2].child_b = 0;
    expected_before_nodes[2].flags = 0xA2000002u;

    expected_after_owner.allocator_owner = replacement_allocator;
    expected_after_owner.list_head =
        (struct ListCountNode *)(uintptr_t)0x55556666u;
    expected_after_owner.tree_root = &nodes[2];
    expected_after_nodes[0].unknown00 = 0xD0000000u;
    expected_after_nodes[0].unknown04 = 0xD0000001u;
    expected_after_nodes[0].parent = &nodes[2];
    expected_after_nodes[0].child_a = &nodes[1];
    expected_after_nodes[0].child_b = 0;
    expected_after_nodes[0].flags = 0xD0000002u;
    expected_after_nodes[1].unknown00 = 0xD1000000u;
    expected_after_nodes[1].unknown04 = 0xD1000001u;
    expected_after_nodes[1].parent = 0;
    expected_after_nodes[1].child_a = &nodes[0];
    expected_after_nodes[1].child_b = &nodes[2];
    expected_after_nodes[1].flags = 0xD1000002u;
    expected_after_nodes[2].unknown00 = 0xD2000000u;
    expected_after_nodes[2].unknown04 = 0xD2000001u;
    expected_after_nodes[2].parent = &nodes[1];
    expected_after_nodes[2].child_a = 0;
    expected_after_nodes[2].child_b = &nodes[0];
    expected_after_nodes[2].flags = 0xD2000002u;

    trace_count = 0;
    DetachAndFreeTreeNode(&owner, &nodes[1]);
    assert(trace_count == 2);
    assert(trace[0] == TRACE_DETACH);
    assert(trace[1] == TRACE_FREE);
    assert(memcmp(&owner, &expected_after_owner, sizeof owner) == 0);
    assert(memcmp(nodes, expected_after_nodes, sizeof nodes) == 0);
}

int main(void)
{
    run_wrapper_mutation_case();
    return 0;
}
