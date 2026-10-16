#include <assert.h>
#include <string.h>

#include "../../include/allocator_wrappers.h"
#include "../../include/list_append.h"

#define NODE_COUNT 6

static struct Container owner;
static struct Container expected_owner;
static struct ListCountNode nodes[NODE_COUNT];
static struct ListCountNode expected_nodes[NODE_COUNT];
static struct ListCountNode allocation;
static struct ListCountNode expected_allocation;
static struct ListCountNode alternate[2];
static struct ListCountNode expected_alternate[2];
static void *expected_allocator;
static u32 expected_payload;
static struct ListCountNode *head_during_allocation;
static unsigned allocation_calls;

void *AllocatorAllocate(void *allocator_owner, u32 size)
{
    assert(allocation_calls == 0);
    assert(allocator_owner == expected_allocator);
    assert(size == 12);
    owner.list_head = head_during_allocation;
    ++allocation_calls;
    return &allocation;
}

static void run_case(unsigned initial_count, u32 payload,
                     struct ListCountNode *callback_head,
                     unsigned selected_count, unsigned append_at)
{
    unsigned i;
    struct ListCountNode *result;

    memset(&owner, 0xA5, sizeof(owner));
    memset(nodes, 0x5A, sizeof(nodes));
    memset(&allocation, 0xC3, sizeof(allocation));

    for (i = 0; i < initial_count; ++i) {
        nodes[i].unknown00 = 0x10203040u + i;
        nodes[i].previous = i ? &nodes[i - 1] : 0;
        nodes[i].next = i + 1 < initial_count ? &nodes[i + 1] : 0;
    }
    if (initial_count == 3)
        nodes[2].unknown00 = payload;
    for (i = initial_count; i < NODE_COUNT; ++i) {
        nodes[i].unknown00 = 0xD0D00000u + i;
        nodes[i].previous = (struct ListCountNode *)(uintptr_t)(0x11110000u + i);
        nodes[i].next = (struct ListCountNode *)(uintptr_t)(0x22220000u + i);
    }
    owner.allocator_owner = (void *)(uintptr_t)0x12345678u;
    owner.list_head = initial_count ? &nodes[0] : 0;
    owner.tree_root = (struct TreeCountNode *)(uintptr_t)0xABCDEF00u;

    memcpy(expected_nodes, nodes, sizeof(nodes));
    memcpy(expected_alternate, alternate, sizeof(alternate));
    expected_owner = owner;
    expected_owner.list_head = callback_head;
    expected_allocation = allocation;
    expected_allocation.unknown00 = payload;
    expected_allocation.previous = 0;
    expected_allocation.next = 0;

    if (selected_count == 0) {
        expected_owner.list_head = &allocation;
    } else if (callback_head == &alternate[0]) {
        expected_alternate[1].next = &allocation;
        expected_allocation.previous = &alternate[1];
    } else {
        expected_nodes[append_at].next = &allocation;
        expected_allocation.previous = &nodes[append_at];
    }
    expected_allocator = owner.allocator_owner;
    expected_payload = payload;
    head_during_allocation = callback_head;
    allocation_calls = 0;

    result = AppendListNode(&owner, expected_payload);

    assert(allocation_calls == 1);
    assert(result == &allocation);
    assert(memcmp(&owner, &expected_owner, sizeof(owner)) == 0);
    assert(memcmp(nodes, expected_nodes, sizeof(nodes)) == 0);
    assert(memcmp(alternate, expected_alternate, sizeof(alternate)) == 0);
    assert(memcmp(&allocation, &expected_allocation, sizeof(allocation)) == 0);
}

int main(void)
{
    /* Empty list; payload zero must still be stored and returned. */
    run_case(0, 0, 0, 0, 0);

    /* One-node list and high-bit payload. */
    run_case(1, 0x80000001u, &nodes[0], 1, 0);

    /* Three-node list; repeated payload values are not special. */
    run_case(3, 0xA5A5A5A5u, &nodes[0], 3, 2);
    run_case(3, 0xA5A5A5A5u, &nodes[0], 3, 2);

    /* Allocator callback replaces owner head; append must reload it afterward. */
    memset(alternate, 0, sizeof(alternate));
    alternate[0].unknown00 = 0x31415926u;
    alternate[0].next = &alternate[1];
    alternate[1].unknown00 = 0x27182818u;
    alternate[1].previous = &alternate[0];
    run_case(1, 0xFEDCBA98u, &alternate[0], 2, 0);
    return 0;
}
