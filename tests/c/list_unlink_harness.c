#include <assert.h>
#include <string.h>

#include "../../include/list_unlink.h"
#include "../../include/allocator_wrappers.h"

#define NODE_COUNT 4

static struct Container owner;
static struct Container expected_owner;
static struct ListCountNode nodes[NODE_COUNT];
static struct ListCountNode expected_nodes[NODE_COUNT];
static unsigned calls;
static void *expected_allocator;
static struct ListCountNode *expected_allocation;

void AllocatorFree(void *allocator_owner, void *allocation)
{
    assert(calls == 0);
    assert(allocator_owner == expected_allocator);
    assert(allocation == expected_allocation);
    assert(memcmp(&owner, &expected_owner, sizeof(owner)) == 0);
    assert(memcmp(nodes, expected_nodes, sizeof(nodes)) == 0);
    ++calls;
}

static void run_case(unsigned count, unsigned removed_index)
{
    unsigned i;
    struct ListCountNode *removed;

    memset(&owner, 0xA5, sizeof(owner));
    memset(nodes, 0x5A, sizeof(nodes));
    for (i = 0; i < count; ++i) {
        nodes[i].unknown00 = 0x10203040u + i;
        nodes[i].previous = i ? &nodes[i - 1] : 0;
        nodes[i].next = i + 1 < count ? &nodes[i + 1] : 0;
    }
    owner.allocator_owner = (void *)0x12345678u;
    owner.list_head = count ? &nodes[0] : 0;
    owner.tree_root = (struct TreeCountNode *)0xABCDEF00u;
    removed = &nodes[removed_index];

    memcpy(expected_nodes, nodes, sizeof(nodes));
    expected_owner = owner;
    if (removed_index == 0)
        expected_owner.list_head = removed->next;
    else
        expected_nodes[removed_index - 1].next = removed->next;
    if (removed_index + 1 < count)
        expected_nodes[removed_index + 1].previous = removed->previous;

    expected_allocator = owner.allocator_owner;
    expected_allocation = removed;
    calls = 0;

    UnlinkAndFreeListNode(&owner, removed);

    assert(calls == 1);
    assert(memcmp(&owner, &expected_owner, sizeof(owner)) == 0);
    assert(memcmp(nodes, expected_nodes, sizeof(nodes)) == 0);
}

int main(void)
{
    run_case(1, 0); /* sole node: head update, no successor update */
    run_case(4, 0); /* head: head update and successor previous update */
    run_case(4, 1); /* middle: predecessor and successor updates */
    run_case(4, 3); /* tail: predecessor update, no successor update */
    return 0;
}
