#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "../../include/container_destroy.h"
#include "../../include/list_count.h"
#include "../../include/list_free.h"
#include "../../include/tree_count.h"

#define LISTS 3
#define TREES 4
#define CALLS 8
#define TOKEN_A ((void *)0x11110000)
#define TOKEN_B ((void *)0x22220000)
#define TOKEN_C ((void *)0x33330000)
#define TOKEN_D ((void *)0x44440000)

struct Image {
    struct Container owner;
    struct ListCountNode lists[LISTS];
    struct TreeCountNode trees[TREES];
};

static struct Container owner;
static struct ListCountNode lists[LISTS];
static struct TreeCountNode trees[TREES];
static struct TreeCountNode decoy;
static struct Container expected_owner[CALLS];
static struct ListCountNode expected_lists[CALLS][LISTS];
static struct TreeCountNode expected_trees[CALLS][TREES];
static struct TreeCountNode expected_decoy[CALLS];
static void *expected_allocators[CALLS];
static void *expected_allocations[CALLS];
static unsigned expected_count;
static unsigned call_count;
static unsigned mutate_list_call;
static unsigned mutate_tree_call;
static unsigned mutate_final_call;
static int run_combined_mutation;

static void snapshot(unsigned index)
{
    expected_owner[index] = owner;
    memcpy(expected_lists[index], lists, sizeof(lists));
    memcpy(expected_trees[index], trees, sizeof(trees));
    expected_decoy[index] = decoy;
}

void AllocatorFree(void *allocator_owner, void *allocation)
{
    unsigned call = call_count;

    assert(call < expected_count);
    assert(allocator_owner == expected_allocators[call]);
    assert(allocation == expected_allocations[call]);
    assert(memcmp(&owner, &expected_owner[call], sizeof(owner)) == 0);
    assert(memcmp(lists, expected_lists[call], sizeof(lists)) == 0);
    assert(memcmp(trees, expected_trees[call], sizeof(trees)) == 0);
    assert(memcmp(&decoy, &expected_decoy[call], sizeof(decoy)) == 0);
    call_count++;

    if (run_combined_mutation && call == mutate_list_call)
        owner.allocator_owner = TOKEN_B;
    if (run_combined_mutation && call == mutate_tree_call) {
        owner.allocator_owner = TOKEN_C;
        trees[1].parent = &decoy;
        trees[0].child_a = &decoy;
    }
    if (run_combined_mutation && call == mutate_final_call)
        owner.allocator_owner = TOKEN_D;
}

static void reset(void)
{
    memset(&owner, 0xA5, sizeof(owner));
    memset(lists, 0x5A, sizeof(lists));
    memset(trees, 0xC3, sizeof(trees));
    memset(&decoy, 0xD4, sizeof(decoy));
    memset(expected_owner, 0, sizeof(expected_owner));
    memset(expected_lists, 0, sizeof(expected_lists));
    memset(expected_trees, 0, sizeof(expected_trees));
    memset(expected_decoy, 0, sizeof(expected_decoy));
    memset(expected_allocators, 0, sizeof(expected_allocators));
    memset(expected_allocations, 0, sizeof(expected_allocations));
    owner.allocator_owner = TOKEN_A;
    owner.list_head = 0;
    owner.tree_root = 0;
    expected_count = 0;
    call_count = 0;
    mutate_list_call = (unsigned)-1;
    mutate_tree_call = (unsigned)-1;
    mutate_final_call = (unsigned)-1;
    run_combined_mutation = 0;
}

static void set_list(unsigned count)
{
    unsigned i;
    for (i = 0; i < count; i++) {
        lists[i].previous = i ? &lists[i - 1] : 0;
        lists[i].next = i + 1 < count ? &lists[i + 1] : 0;
    }
    owner.list_head = count ? &lists[0] : 0;
}

static void set_tree_three(void)
{
    owner.tree_root = &trees[0];
    trees[0].parent = 0;
    trees[0].child_a = &trees[1];
    trees[0].child_b = &trees[2];
    trees[1].parent = &trees[0];
    trees[1].child_a = 0;
    trees[1].child_b = 0;
    trees[2].parent = &trees[0];
    trees[2].child_a = 0;
    trees[2].child_b = 0;
}

static void test_empty(void)
{
    reset();
    snapshot(0);
    expected_count = 1;
    expected_allocators[0] = TOKEN_A;
    expected_allocations[0] = &owner;
    {
        struct Container final_owner = owner;
        struct ListCountNode final_lists[LISTS];
        struct TreeCountNode final_trees[TREES];
        struct TreeCountNode final_decoy = decoy;
        void *returned;
        final_owner.list_head = 0;
        final_owner.tree_root = 0;
        memcpy(final_lists, lists, sizeof(lists));
        memcpy(final_trees, trees, sizeof(trees));
        returned = DestroyContainer(&owner);
        assert(returned == TOKEN_A);
        assert(call_count == 1);
        assert(memcmp(&owner, &final_owner, sizeof(owner)) == 0);
        assert(memcmp(lists, final_lists, sizeof(lists)) == 0);
        assert(memcmp(trees, final_trees, sizeof(trees)) == 0);
        assert(memcmp(&decoy, &final_decoy, sizeof(decoy)) == 0);
    }
}

static void test_list_only(void)
{
    void *trace[] = { &lists[2], &lists[1], &lists[0], &owner };
    struct ListCountNode final_lists[LISTS];
    struct TreeCountNode final_trees[TREES];
    struct TreeCountNode final_decoy;
    unsigned i;
    reset();
    set_list(LISTS);
    memcpy(final_lists, lists, sizeof(lists));
    memcpy(final_trees, trees, sizeof(trees));
    final_decoy = decoy;
    expected_count = 4;
    for (i = 0; i < expected_count; i++) {
        snapshot(i);
        expected_allocators[i] = TOKEN_A;
        expected_allocations[i] = trace[i];
    }
    expected_owner[3].list_head = 0;
    {
        struct Container final_owner = owner;
        void *returned;
        final_owner.list_head = 0;
        final_owner.tree_root = 0;
        returned = DestroyContainer(&owner);
        assert(returned == TOKEN_A);
        assert(call_count == expected_count);
        assert(memcmp(&owner, &final_owner, sizeof(owner)) == 0);
    }
    assert(memcmp(lists, final_lists, sizeof(lists)) == 0);
    assert(memcmp(trees, final_trees, sizeof(trees)) == 0);
    assert(memcmp(&decoy, &final_decoy, sizeof(decoy)) == 0);
}

static void test_tree_only(void)
{
    void *trace[] = { &trees[1], &trees[2], &trees[0], &owner };
    struct ListCountNode final_lists[LISTS];
    struct TreeCountNode final_trees[TREES];
    struct TreeCountNode final_decoy;
    unsigned i;
    reset();
    set_tree_three();
    memcpy(final_lists, lists, sizeof(lists));
    memcpy(final_trees, trees, sizeof(trees));
    final_decoy = decoy;
    expected_count = 4;
    for (i = 0; i < expected_count; i++) {
        snapshot(i);
        expected_allocators[i] = TOKEN_A;
        expected_allocations[i] = trace[i];
    }
    expected_owner[3].tree_root = 0;
    {
        struct Container final_owner = owner;
        void *returned;
        final_owner.list_head = 0;
        final_owner.tree_root = 0;
        returned = DestroyContainer(&owner);
        assert(returned == TOKEN_A);
        assert(call_count == expected_count);
        assert(memcmp(&owner, &final_owner, sizeof(owner)) == 0);
    }
    assert(memcmp(lists, final_lists, sizeof(lists)) == 0);
    assert(memcmp(trees, final_trees, sizeof(trees)) == 0);
    assert(memcmp(&decoy, &final_decoy, sizeof(decoy)) == 0);
}

static void test_combined_mutation_and_postfree_parent_reload(void)
{
    void *trace[] = {
        &lists[1], &lists[0], &trees[1], &decoy, &trees[0], &owner
    };
    struct ListCountNode final_lists[LISTS];
    struct TreeCountNode final_trees[TREES];
    struct TreeCountNode final_decoy;
    unsigned i;

    reset();
    set_list(2);
    owner.tree_root = &trees[0];
    trees[0].parent = 0;
    trees[0].child_a = &trees[1];
    trees[0].child_b = 0;
    trees[1].parent = &trees[0];
    trees[1].child_a = 0;
    trees[1].child_b = 0;
    decoy.parent = &trees[0];
    decoy.child_a = &trees[1];
    decoy.child_b = 0;

    expected_count = 6;
    for (i = 0; i < expected_count; i++) {
        snapshot(i);
        expected_allocations[i] = trace[i];
    }
    expected_allocators[0] = TOKEN_A;
    expected_allocators[1] = TOKEN_A;
    expected_allocators[2] = TOKEN_B;
    expected_allocators[3] = TOKEN_B;
    expected_allocators[4] = TOKEN_B;
    expected_allocators[5] = TOKEN_C;

    /* State at list callback 1 follows the first callback's A->B mutation. */
    expected_owner[1].allocator_owner = TOKEN_B;
    /* The list root is cleared after its callbacks, before the tree walk. */
    expected_owner[2].allocator_owner = TOKEN_B;
    expected_owner[2].list_head = 0;
    /* First tree callback still sees B and the original roots/node links. */
    expected_owner[3].allocator_owner = TOKEN_C;
    expected_owner[3].list_head = 0;
    expected_trees[3][1].parent = &decoy;
    expected_trees[3][0].child_a = &decoy;
    expected_decoy[3] = decoy;
    expected_owner[4] = expected_owner[3];
    expected_trees[4][0] = expected_trees[3][0];
    expected_trees[4][1] = expected_trees[3][1];
    expected_decoy[4] = expected_decoy[3];
    expected_owner[5] = expected_owner[4];
    expected_owner[5].tree_root = 0;
    expected_trees[5][0] = expected_trees[4][0];
    expected_trees[5][1] = expected_trees[4][1];
    expected_decoy[5] = expected_decoy[4];

    memcpy(final_lists, lists, sizeof(lists));
    memcpy(final_trees, trees, sizeof(trees));
    final_trees[1].parent = &decoy;
    final_trees[0].child_a = &decoy;
    final_decoy = decoy;
    run_combined_mutation = 1;
    mutate_list_call = 0;
    mutate_tree_call = 2;
    mutate_final_call = 5;

    {
        struct Container final_owner = expected_owner[5];
        void *returned;
        final_owner.allocator_owner = TOKEN_D;
        returned = DestroyContainer(&owner);
        assert(returned == TOKEN_C);
        assert(call_count == expected_count);
        assert(memcmp(&owner, &final_owner, sizeof(owner)) == 0);
    }
    assert(memcmp(lists, final_lists, sizeof(lists)) == 0);
    assert(memcmp(trees, final_trees, sizeof(trees)) == 0);
    assert(memcmp(&decoy, &final_decoy, sizeof(decoy)) == 0);
}

int main(void)
{
    assert(offsetof(struct Container, allocator_owner) == 0);
    assert(offsetof(struct Container, list_head) == sizeof(void *));
    assert(offsetof(struct Container, tree_root) == 2 * sizeof(void *));
    assert(offsetof(struct ListCountNode, next) == 2 * sizeof(void *));
    assert(offsetof(struct TreeCountNode, parent) ==
           (sizeof(void *) == 4 ? 8 : 16));
    assert(offsetof(struct TreeCountNode, child_a) ==
           (sizeof(void *) == 4 ? 12 : 24));
    assert(offsetof(struct TreeCountNode, child_b) ==
           (sizeof(void *) == 4 ? 16 : 32));
    test_empty();
    test_list_only();
    test_tree_only();
    test_combined_mutation_and_postfree_parent_reload();
    return 0;
}
