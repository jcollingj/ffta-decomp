#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "../../include/list_clear.h"
#include "../../include/list_count.h"
#include "../../include/allocator_wrappers.h"
#include "../../include/list_free.h"

#define NODE_COUNT 5

static struct Container owner;
static struct ListCountNode nodes[NODE_COUNT];
static struct ListCountNode *freed[NODE_COUNT];
static unsigned freed_count;
static void *expected_allocator_owner;
static struct ListCountNode *expected_head;
static int require_original_head_during_callbacks;
static int mutate_owner_on_first_callback;

void AllocatorFree(void *allocator_owner, void *allocation)
{
    assert(allocator_owner == expected_allocator_owner);
    assert(freed_count < NODE_COUNT);
    if (require_original_head_during_callbacks)
        assert(owner.list_head == expected_head);
    freed[freed_count++] = (struct ListCountNode *)allocation;
    if (mutate_owner_on_first_callback && freed_count == 1) {
        owner.allocator_owner = (void *)0x87654321;
        owner.list_head = &nodes[4];
    }
}

static void reset(unsigned count)
{
    unsigned i;
    memset(&owner, 0xA5, sizeof(owner));
    memset(nodes, 0x5A, sizeof(nodes));
    for (i = 0; i < NODE_COUNT; ++i)
        nodes[i].previous = 0;
    memset(freed, 0, sizeof(freed));
    expected_allocator_owner = (void *)0x12345678;
    expected_head = count ? &nodes[0] : 0;
    owner.allocator_owner = expected_allocator_owner;
    owner.list_head = expected_head;
    owner.tree_root = (struct TreeCountNode *)0xABCDEF00;
    for (i = 0; i < count; ++i) {
        nodes[i].previous = i ? &nodes[i - 1] : 0;
        nodes[i].next = i + 1 < count ? &nodes[i + 1] : 0;
    }
    freed_count = 0;
    require_original_head_during_callbacks = 1;
    mutate_owner_on_first_callback = 0;
}

static void verify(const unsigned *order, unsigned count)
{
    struct ListCountNode nodes_before[NODE_COUNT];
    struct TreeCountNode *tree_before = owner.tree_root;
    void *allocator_before = owner.allocator_owner;
    unsigned i;
    memcpy(nodes_before, nodes, sizeof(nodes));
    ClearContainerList(&owner);
    assert(freed_count == count);
    for (i = 0; i < count; ++i)
        assert(freed[i] == &nodes[order[i]]);
    assert(owner.allocator_owner == allocator_before);
    assert(owner.list_head == 0);
    assert(owner.tree_root == tree_before);
    assert(memcmp(nodes, nodes_before, sizeof(nodes)) == 0);
}

int main(void)
{
    static const unsigned one[] = {0};
    static const unsigned many[] = {3, 2, 1, 0};
    unsigned none[1] = {0};

    assert(offsetof(struct Container, allocator_owner) == 0);
    assert(offsetof(struct Container, list_head) == sizeof(void *));
    assert(offsetof(struct ListCountNode, previous) ==
           (sizeof(void *) == 8 ? 8 : 4));
    assert(offsetof(struct ListCountNode, next) ==
           (sizeof(void *) == 8 ? 16 : 8));

    reset(0);
    verify(none, 0);

    reset(1);
    verify(one, 1);

    reset(4);
    verify(many, 4);

    /* The first tail callback changes both owner fields. The routine must
       still use the allocator and head captured before entering the callback,
       while preserving storage until all frees have completed. */
    reset(4);
    require_original_head_during_callbacks = 0;
    mutate_owner_on_first_callback = 1;
    ClearContainerList(&owner);
    assert(freed_count == 4);
    assert(freed[0] == &nodes[3]);
    assert(freed[1] == &nodes[2]);
    assert(freed[2] == &nodes[1]);
    assert(freed[3] == &nodes[0]);
    assert(owner.allocator_owner == (void *)0x87654321);
    assert(owner.list_head == 0);
    assert(owner.tree_root == (struct TreeCountNode *)0xABCDEF00);
    return 0;
}
