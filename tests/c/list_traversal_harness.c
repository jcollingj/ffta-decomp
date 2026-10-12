#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "../../include/list_traversal.h"
#include "../../src/list_traversal.c"

#define NODE_COUNT 16
#define NO_STOP NODE_COUNT

static struct TreeCountNode nodes[NODE_COUNT];

struct VisitState {
    void *expected_context;
    unsigned calls;
    unsigned ids[NODE_COUNT];
    unsigned stop_id;
    int continue_result;
    struct TreeCountNode *mutate_at;
    struct TreeCountNode *replacement_child_b;
};

static int record_visit(struct TreeCountNode *node, void *context)
{
    struct VisitState *state = (struct VisitState *)context;

    assert(context == state->expected_context);
    assert(state->calls < NODE_COUNT);
    state->ids[state->calls++] = node->unknown00;
    if (node == state->mutate_at)
        node->child_b = state->replacement_child_b;
    if (node->unknown00 == state->stop_id)
        return 0;
    return state->continue_result;
}

static void reset_nodes(void)
{
    unsigned i;

    memset(nodes, 0, sizeof(nodes));
    for (i = 0; i < NODE_COUNT; i++)
        nodes[i].unknown00 = i;
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

static void init_owner(struct TreeCountOwner *owner,
                       struct TreeCountNode *root)
{
    owner->unknown00 = 0x12345678;
    owner->unknown04 = 0x89ABCDEF;
    owner->root = root;
}

static void build_balanced_tree(void)
{
    reset_nodes();
    link_a(0, 1);
    link_b(0, 2);
    link_a(1, 3);
    link_b(1, 4);
}

static void expect_readonly_visits(const struct TreeCountOwner *owner,
                                   struct VisitState *state,
                                   const unsigned *expected, unsigned count)
{
    struct TreeCountNode nodes_before[NODE_COUNT];
    struct TreeCountOwner owner_before;
    unsigned i;

    memcpy(nodes_before, nodes, sizeof(nodes));
    memcpy(&owner_before, owner, sizeof(owner_before));
    TraverseTreeInOrder(owner, record_visit, state);
    assert(state->calls == count);
    for (i = 0; i < count; i++)
        assert(state->ids[i] == expected[i]);
    assert(memcmp(nodes, nodes_before, sizeof(nodes)) == 0);
    assert(memcmp(owner, &owner_before, sizeof(owner_before)) == 0);
}

static void init_state(struct VisitState *state)
{
    memset(state, 0, sizeof(*state));
    state->expected_context = state;
    state->stop_id = NO_STOP;
    state->continue_result = 2;
}

int main(void)
{
    struct TreeCountOwner owner;
    struct VisitState state;
    const unsigned one[] = {0};
    const unsigned tree_order[] = {3, 1, 4, 0, 2};
    const unsigned replaced_successor_order[] = {1, 0, 5};
    const unsigned subtree_order[] = {7, 5, 8};

    /* Empty-root guard: no callback is invoked, even with a null callback. */
    reset_nodes();
    init_owner(&owner, 0);
    {
        struct TreeCountNode nodes_before[NODE_COUNT];
        struct TreeCountOwner owner_before;

        memcpy(nodes_before, nodes, sizeof(nodes));
        memcpy(&owner_before, &owner, sizeof(owner_before));
        TraverseTreeInOrder(&owner, 0, 0);
        assert(memcmp(nodes, nodes_before, sizeof(nodes)) == 0);
        assert(memcmp(&owner, &owner_before, sizeof(owner)) == 0);
    }

    reset_nodes();
    init_state(&state);
    init_owner(&owner, &nodes[0]);
    expect_readonly_visits(&owner, &state, one, 1);

    /* Balanced shape distinguishes child_a/node/child_b callback order.
       Return value 2 is truthy and must continue through the full trace. */
    build_balanced_tree();
    init_state(&state);
    init_owner(&owner, &nodes[0]);
    expect_readonly_visits(&owner, &state, tree_order, 5);

    /* Exercise stopping at every callback position in the fixed trace. */
    {
        unsigned stop_position;

        for (stop_position = 0;
             stop_position < sizeof(tree_order) / sizeof(tree_order[0]);
             stop_position++) {
            build_balanced_tree();
            init_state(&state);
            state.stop_id = tree_order[stop_position];
            init_owner(&owner, &nodes[0]);
            expect_readonly_visits(&owner, &state, tree_order,
                                   stop_position + 1);
        }
    }

    /* child_b is read after the callback: the replacement is followed and
       the former successor subtree is skipped. The fixture verifies that
       this one intended pointer write is the only memory change. */
    reset_nodes();
    init_state(&state);
    link_a(0, 1);
    link_b(0, 2);
    nodes[5].parent = &nodes[0];
    state.mutate_at = &nodes[0];
    state.replacement_child_b = &nodes[5];
    init_owner(&owner, &nodes[0]);
    {
        struct TreeCountNode expected_nodes[NODE_COUNT];
        struct TreeCountOwner owner_before;
        unsigned i;

        memcpy(expected_nodes, nodes, sizeof(nodes));
        expected_nodes[0].child_b = &nodes[5];
        memcpy(&owner_before, &owner, sizeof(owner_before));
        TraverseTreeInOrder(&owner, record_visit, &state);
        assert(state.calls == 3);
        for (i = 0; i < 3; i++)
            assert(state.ids[i] == replaced_successor_order[i]);
        assert(memcmp(nodes, expected_nodes, sizeof(nodes)) == 0);
        assert(memcmp(&owner, &owner_before, sizeof(owner)) == 0);
    }

    /* Starting at a non-root node never visits its external parent/sibling. */
    reset_nodes();
    init_state(&state);
    link_a(0, 5);
    link_b(0, 6);
    link_a(5, 7);
    link_b(5, 8);
    init_owner(&owner, &nodes[5]);
    expect_readonly_visits(&owner, &state, subtree_order, 3);

    return 0;
}
