#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "tree_insert.h"

#define MAX_NODES 8
#define ROOT_INSERT ((size_t)-1)
#define CHILD_LEFT 1
#define CHILD_RIGHT 2

static struct Container owner;
static struct Container expected_owner;
static struct TreeCountNode arena[MAX_NODES];
static struct TreeCountNode expected_arena[MAX_NODES];
static struct TreeCountNode replacement_root;
static struct TreeCountNode expected_replacement_root;
static unsigned allocation_calls;
static unsigned expected_calls;
static u32 allocation_size;
static int mutate_root_on_allocate;

void *AllocatorAllocate(void *allocator, u32 size)
{
    assert(allocator == (void *)0x12345678);
    assert(size == 24);
    assert(size == allocation_size);
    assert(allocation_calls + 1 == expected_calls);
    assert(memcmp(&owner, &expected_owner, sizeof(owner)) == 0);
    allocation_calls++;
    if (mutate_root_on_allocate) {
        owner.tree_root = &replacement_root;
        expected_owner.tree_root = &replacement_root;
    }
    assert(allocation_calls <= MAX_NODES);
    return &arena[allocation_calls - 1];
}

static void assert_all_images(void)
{
    assert(memcmp(&owner, &expected_owner, sizeof(owner)) == 0);
    assert(memcmp(arena, expected_arena, sizeof(arena)) == 0);
    assert(memcmp(&replacement_root, &expected_replacement_root,
                  sizeof(replacement_root)) == 0);
}

static struct TreeCountNode *perform_insert(int key, u32 payload,
                                            size_t parent_index,
                                            int child_side, int mutate_root)
{
    unsigned index = allocation_calls;
    struct TreeCountNode *actual_parent = 0;
    struct TreeCountNode *expected_node;
    struct TreeCountNode *actual_node;

    assert(index < MAX_NODES);
    expected_calls = allocation_calls + 1;
    allocation_size = 24;
    mutate_root_on_allocate = mutate_root;
    actual_node = InsertTreeNode(&owner, payload, key);
    assert(allocation_calls == expected_calls);
    mutate_root_on_allocate = 0;
    assert(actual_node == &arena[index]);
    expected_node = &arena[index];

    expected_arena[index].unknown00 = (u32)key;
    expected_arena[index].unknown04 = payload;
    expected_arena[index].parent = 0;
    expected_arena[index].child_a = 0;
    expected_arena[index].child_b = 0;
    expected_arena[index].flags = 0;

    if (parent_index == ROOT_INSERT) {
        expected_owner.tree_root = expected_node;
    } else if (parent_index == MAX_NODES) {
        actual_parent = &replacement_root;
    } else {
        actual_parent = &arena[parent_index];
    }

    if (actual_parent != 0) {
        expected_arena[index].parent = actual_parent;
        if (child_side == CHILD_LEFT) {
            if (parent_index == MAX_NODES)
                expected_replacement_root.child_a = expected_node;
            else
                expected_arena[parent_index].child_a = expected_node;
        } else {
            assert(child_side == CHILD_RIGHT);
            expected_arena[parent_index].child_b = expected_node;
        }
    }
    assert_all_images();
    return actual_node;
}

int main(void)
{
    struct TreeCountNode *root, *minimum, *maximum, *deep, *equal;

    memset(&owner, 0xA5, sizeof(owner));
    memset(&expected_owner, 0xA5, sizeof(expected_owner));
    memset(arena, 0x5A, sizeof(arena));
    memset(expected_arena, 0x5A, sizeof(expected_arena));
    memset(&replacement_root, 0xC3, sizeof(replacement_root));
    memset(&expected_replacement_root, 0xC3, sizeof(expected_replacement_root));
    owner.allocator_owner = (void *)0x12345678;
    expected_owner.allocator_owner = (void *)0x12345678;
    owner.list_head = (struct ListCountNode *)0x23456789;
    expected_owner.list_head = (struct ListCountNode *)0x23456789;
    owner.tree_root = 0;
    expected_owner.tree_root = 0;
    allocation_calls = 0;
    expected_calls = 0;
    assert_all_images();

    root = perform_insert(0, 0x11223344, ROOT_INSERT, 0, 0);
    minimum = perform_insert((-2147483647 - 1), 0x55667788, 0,
                             CHILD_LEFT, 0);
    maximum = perform_insert(2147483647, 0x99AABBCC, 0,
                             CHILD_RIGHT, 0);
    deep = perform_insert(-7, 0xDEADBEEF, 1, CHILD_RIGHT, 0);
    equal = perform_insert(0, 0xCAFEBABE, 2, CHILD_LEFT, 0);

    assert(root == &arena[0]);
    assert(minimum == &arena[1]);
    assert(maximum == &arena[2]);
    assert(deep == &arena[3]);
    assert(equal == &arena[4]);

    /* Allocation changes the owner root; the routine must use the new root. */
    memset(&owner, 0xA5, sizeof(owner));
    memset(&expected_owner, 0xA5, sizeof(expected_owner));
    memset(arena, 0x5A, sizeof(arena));
    memset(expected_arena, 0x5A, sizeof(expected_arena));
    memset(&replacement_root, 0xC3, sizeof(replacement_root));
    memset(&expected_replacement_root, 0xC3, sizeof(expected_replacement_root));
    owner.allocator_owner = expected_owner.allocator_owner = (void *)0x12345678;
    owner.list_head = expected_owner.list_head = (struct ListCountNode *)0x23456789;
    owner.tree_root = expected_owner.tree_root = 0;
    replacement_root.unknown00 = expected_replacement_root.unknown00 = 100;
    replacement_root.parent = expected_replacement_root.parent = 0;
    replacement_root.child_a = expected_replacement_root.child_a = 0;
    replacement_root.child_b = expected_replacement_root.child_b = 0;
    allocation_calls = 0;
    expected_calls = 0;
    assert_all_images();
    root = perform_insert(5, 7, ROOT_INSERT, 0, 0);
    expected_owner.tree_root = root;
    assert_all_images();
    perform_insert(3, 8, MAX_NODES, CHILD_LEFT, 1);
    assert(expected_owner.tree_root == &replacement_root);
    assert_all_images();
    return 0;
}
