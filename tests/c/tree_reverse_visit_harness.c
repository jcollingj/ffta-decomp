#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "../../include/tree_reverse_visit.h"
#include "../../src/tree_reverse_visit.c"

#define NODE_COUNT 12

static struct TreeCountNode nodes[NODE_COUNT];
static struct TreeCountNode decoys[2];
static struct Container owner;

struct Trace {
    unsigned values[16];
    unsigned length;
    unsigned stop_at;
    int callback_value;
};

static void reset(void)
{
    unsigned i;
    memset(nodes, 0xA5, sizeof(nodes));
    memset(decoys, 0xD7, sizeof(decoys));
    memset(&owner, 0xE1, sizeof(owner));
    for (i = 0; i < NODE_COUNT; i++) {
        nodes[i].parent = 0;
        nodes[i].child_a = 0;
        nodes[i].child_b = 0;
    }
    owner.tree_root = 0;
}

static void link_a(unsigned p, unsigned c)
{
    nodes[p].child_a = &nodes[c];
    nodes[c].parent = &nodes[p];
}

static void link_b(unsigned p, unsigned c)
{
    nodes[p].child_b = &nodes[c];
    nodes[c].parent = &nodes[p];
}

static int trace_callback(void *value, void *opaque)
{
    struct Trace *trace = (struct Trace *)opaque;
    struct TreeCountNode *node = (struct TreeCountNode *)value;
    unsigned index = (unsigned)(node - nodes);
    unsigned position = trace->length++;
    trace->values[position] = index;
    if (position == trace->stop_at)
        return trace->callback_value;
    return trace->callback_value == 0 ? 1 : trace->callback_value;
}

static void assert_trace(const struct Trace *trace, const unsigned *expected,
                         unsigned length)
{
    assert(trace->length == length);
    assert(memcmp(trace->values, expected, length * sizeof(expected[0])) == 0);
}

static void run_fixed_trace(void)
{
    static const unsigned expected[] = { 7, 3, 8, 1, 9, 4, 0, 5, 10, 2, 11, 6 };
    struct TreeCountNode nodes_before[NODE_COUNT];
    struct TreeCountNode decoys_before[2];
    struct Container owner_before;
    struct Trace trace;

    reset();
    owner.tree_root = &nodes[0];
    link_b(0, 1); link_a(0, 2);
    link_b(1, 3); link_a(1, 4);
    link_b(2, 5); link_a(2, 6);
    link_b(3, 7); link_a(3, 8);
    link_b(4, 9); link_a(5, 10);
    link_b(6, 11);
    memset(&trace, 0, sizeof(trace));
    trace.stop_at = 99;
    trace.callback_value = 1;
    memcpy(nodes_before, nodes, sizeof(nodes));
    memcpy(decoys_before, decoys, sizeof(decoys));
    owner_before = owner;
    TraverseTreeReverseInOrder(&owner, trace_callback, &trace);
    assert_trace(&trace, expected, sizeof(expected) / sizeof(expected[0]));
    assert(memcmp(nodes, nodes_before, sizeof(nodes)) == 0);
    assert(memcmp(decoys, decoys_before, sizeof(decoys)) == 0);
    assert(memcmp(&owner, &owner_before, sizeof(owner)) == 0);
}

static void run_stop_positions(void)
{
    static const unsigned expected[] = { 7, 3, 8, 1, 9, 4, 0, 5, 10, 2, 11, 6 };
    unsigned stop;

    for (stop = 0; stop < sizeof(expected) / sizeof(expected[0]); stop++) {
        struct TreeCountNode nodes_before[NODE_COUNT];
        struct TreeCountNode decoys_before[2];
        struct Container owner_before;
        struct Trace trace;

        reset();
        owner.tree_root = &nodes[0];
        link_b(0, 1); link_a(0, 2);
        link_b(1, 3); link_a(1, 4);
        link_b(2, 5); link_a(2, 6);
        link_b(3, 7); link_a(3, 8);
        link_b(4, 9); link_a(5, 10);
        link_b(6, 11);
        memset(&trace, 0, sizeof(trace));
        trace.stop_at = stop;
        trace.callback_value = 0;
        memcpy(nodes_before, nodes, sizeof(nodes));
        memcpy(decoys_before, decoys, sizeof(decoys));
        owner_before = owner;
        TraverseTreeReverseInOrder(&owner, trace_callback, &trace);
        assert_trace(&trace, expected, stop + 1);
        assert(memcmp(nodes, nodes_before, sizeof(nodes)) == 0);
        assert(memcmp(decoys, decoys_before, sizeof(decoys)) == 0);
        assert(memcmp(&owner, &owner_before, sizeof(owner)) == 0);
    }
}

struct MutationTrace {
    unsigned values[8];
    unsigned length;
};

static int mutate_successor(void *value, void *opaque)
{
    struct MutationTrace *trace = (struct MutationTrace *)opaque;
    struct TreeCountNode *node = (struct TreeCountNode *)value;
    trace->values[trace->length++] = (unsigned)(node - nodes);
    if (node == &nodes[1])
        node->child_a = &nodes[5];
    return 7;
}

static void run_successor_mutation(void)
{
    static const unsigned actual_expected[] = { 3, 1, 5, 0, 2 };
    struct TreeCountNode expected_nodes[NODE_COUNT];
    struct TreeCountNode decoys_before[2];
    struct Container owner_before;
    struct MutationTrace trace;

    reset();
    owner.tree_root = &nodes[0];
    link_b(0, 1); link_a(0, 2);
    link_b(1, 3); link_a(1, 4);
    nodes[5].parent = &nodes[1];
    memcpy(expected_nodes, nodes, sizeof(nodes));
    expected_nodes[1].child_a = &nodes[5];
    memcpy(decoys_before, decoys, sizeof(decoys));
    owner_before = owner;
    memset(&trace, 0, sizeof(trace));
    TraverseTreeReverseInOrder(&owner, mutate_successor, &trace);
    assert(trace.length == sizeof(actual_expected) / sizeof(actual_expected[0]));
    assert(memcmp(trace.values, actual_expected, sizeof(actual_expected)) == 0);
    assert(memcmp(nodes, expected_nodes, sizeof(nodes)) == 0);
    assert(memcmp(decoys, decoys_before, sizeof(decoys)) == 0);
    assert(memcmp(&owner, &owner_before, sizeof(owner)) == 0);
}

struct ConstantTrace { unsigned calls; };

static int nonzero_callback(void *value, void *opaque)
{
    struct ConstantTrace *trace = (struct ConstantTrace *)opaque;
    (void)value;
    trace->calls++;
    return 2;
}

static void run_null_and_subtree(void)
{
    static const unsigned subtree_expected[] = { 3, 1, 4 };
    struct TreeCountNode nodes_before[NODE_COUNT];
    struct TreeCountNode decoys_before[2];
    struct Container owner_before;
    struct ConstantTrace null_trace;
    struct Trace subtree_trace;

    reset();
    owner.tree_root = 0;
    null_trace.calls = 0;
    TraverseTreeReverseInOrder(&owner, nonzero_callback, &null_trace);
    assert(null_trace.calls == 0);

    reset();
    owner.tree_root = &nodes[1];
    link_b(0, 1); link_a(0, 2);
    link_b(1, 3); link_a(1, 4);
    link_b(2, 5); link_a(2, 6);
    memcpy(nodes_before, nodes, sizeof(nodes));
    memcpy(decoys_before, decoys, sizeof(decoys));
    owner_before = owner;
    memset(&subtree_trace, 0, sizeof(subtree_trace));
    subtree_trace.stop_at = 99;
    subtree_trace.callback_value = 2;
    TraverseTreeReverseInOrder(&owner, trace_callback, &subtree_trace);
    assert_trace(&subtree_trace, subtree_expected, 3);
    assert(memcmp(nodes, nodes_before, sizeof(nodes)) == 0);
    assert(memcmp(decoys, decoys_before, sizeof(decoys)) == 0);
    assert(memcmp(&owner, &owner_before, sizeof(owner)) == 0);
}

int main(void)
{
    run_fixed_trace();
    run_stop_positions();
    run_successor_mutation();
    run_null_and_subtree();
    return 0;
}
