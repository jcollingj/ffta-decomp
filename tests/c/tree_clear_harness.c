#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "../../include/tree_clear.h"
#include "../../include/tree_count.h"

#define MAX_NODES 16

static struct Container owner;
static struct TreeCountNode nodes[MAX_NODES];
static struct TreeCountNode decoy;
static struct TreeCountNode *freed[MAX_NODES];
static struct TreeCountNode *expected_trace[MAX_NODES];
static struct Container owner_at_call;
static struct TreeCountNode callback_nodes[MAX_NODES];
static struct TreeCountNode callback_decoy;
static struct TreeCountNode final_nodes[MAX_NODES];
static struct TreeCountNode final_decoy;
static unsigned freed_count;
static unsigned expected_count;
static struct TreeCountNode *redirect_from;
static struct TreeCountNode *redirect_to;

void AllocatorFree(void *allocator_owner, void *allocation)
{
    unsigned i;
    struct TreeCountNode *node = allocation;

    assert(memcmp(&owner, &owner_at_call, sizeof(owner)) == 0);
    assert(memcmp(nodes, callback_nodes, sizeof(nodes)) == 0);
    assert(memcmp(&decoy, &callback_decoy, sizeof(decoy)) == 0);
    assert(allocator_owner == (void *)0x12345678);
    assert(freed_count < expected_count);
    assert(allocation == expected_trace[freed_count]);
    freed[freed_count++] = allocation;

    if (node == redirect_from) {
        node->parent = redirect_to;
        for (i = 0; i < MAX_NODES; i++) {
            if (node == &nodes[i]) {
                callback_nodes[i].parent = redirect_to;
                break;
            }
        }
        if (node == &decoy)
            callback_decoy.parent = redirect_to;
    }
}

static void reset(unsigned count)
{
    unsigned i;

    memset(&owner, 0xA5, sizeof(owner));
    memset(nodes, 0x5A, sizeof(nodes));
    memset(&decoy, 0xC3, sizeof(decoy));
    memset(freed, 0, sizeof(freed));
    memset(expected_trace, 0, sizeof(expected_trace));
    owner.allocator_owner = (void *)0x12345678;
    owner.list_head = (struct ListCountNode *)0x87654321;
    owner.tree_root = count ? &nodes[0] : 0;
    for (i = 0; i < count; i++) {
        nodes[i].parent = 0;
        nodes[i].child_a = 0;
        nodes[i].child_b = 0;
    }
    freed_count = 0;
    expected_count = 0;
    redirect_from = 0;
    redirect_to = 0;
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

static void verify(const unsigned *expected, unsigned count)
{
    unsigned i;

    expected_count = count;
    for (i = 0; i < count; i++)
        expected_trace[i] = &nodes[expected[i]];
    owner_at_call = owner;
    memcpy(callback_nodes, nodes, sizeof(nodes));
    callback_decoy = decoy;
    memcpy(final_nodes, nodes, sizeof(nodes));
    final_decoy = decoy;

    FreeTreeNodes(&owner);
    assert(freed_count == count);
    for (i = 0; i < count; i++)
        assert(freed[i] == expected_trace[i]);
    assert(owner.allocator_owner == owner_at_call.allocator_owner);
    assert(owner.list_head == owner_at_call.list_head);
    assert(owner.tree_root == 0);
    assert(memcmp(nodes, final_nodes, sizeof(nodes)) == 0);
    assert(memcmp(&decoy, &final_decoy, sizeof(decoy)) == 0);
}

static void verify_redirected_parent(void)
{
    owner_at_call = owner;
    expected_count = 2;
    expected_trace[0] = &nodes[1];
    expected_trace[1] = &decoy;
    memcpy(callback_nodes, nodes, sizeof(nodes));
    callback_decoy = decoy;

    /* Construct the complete post-call oracle on separate storage. */
    memcpy(final_nodes, nodes, sizeof(nodes));
    final_nodes[1].parent = &decoy;
    final_decoy = decoy;
    redirect_from = &nodes[1];
    redirect_to = &decoy;

    FreeTreeNodes(&owner);
    assert(freed_count == 2);
    assert(freed[0] == expected_trace[0]);
    assert(freed[1] == expected_trace[1]);
    assert(owner.allocator_owner == owner_at_call.allocator_owner);
    assert(owner.list_head == owner_at_call.list_head);
    assert(owner.tree_root == 0);
    assert(memcmp(nodes, final_nodes, sizeof(nodes)) == 0);
    assert(memcmp(&decoy, &final_decoy, sizeof(decoy)) == 0);
}

int main(void)
{
    static const unsigned single[] = {0};
    static const unsigned asymmetric[] = {3, 1, 4, 2, 0};
    static const unsigned balanced[] = {3, 4, 1, 5, 6, 2, 0};
    struct Container empty_before;

    assert(offsetof(struct TreeCountNode, parent) ==
           (sizeof(void *) == 4 ? 8 : 16));
    assert(offsetof(struct TreeCountNode, child_a) ==
           (sizeof(void *) == 4 ? 12 : 24));
    assert(offsetof(struct TreeCountNode, child_b) ==
           (sizeof(void *) == 4 ? 16 : 32));
    assert(offsetof(struct Container, tree_root) == 2 * sizeof(void *));

    reset(0);
    empty_before = owner;
    owner_at_call = owner;
    FreeTreeNodes(&owner);
    assert(freed_count == 0);
    assert(owner.allocator_owner == empty_before.allocator_owner);
    assert(owner.list_head == empty_before.list_head);
    assert(owner.tree_root == 0);

    reset(1);
    verify(single, 1);

    reset(5);
    link_a(0, 1); link_b(0, 2);
    link_a(1, 3);
    link_b(2, 4);
    verify(asymmetric, 5);

    reset(7);
    link_a(0, 1); link_b(0, 2);
    link_a(1, 3); link_b(1, 4);
    link_a(2, 5); link_b(2, 6);
    verify(balanced, 7);

    /* The free stub retains storage and mutates node 1's parent. The routine
       must reload that link after the callback and visit the decoy next. */
    reset(3);
    link_a(0, 1);
    link_b(0, 2);
    decoy.parent = 0;
    decoy.child_a = 0;
    decoy.child_b = 0;
    verify_redirected_parent();
    return 0;
}
