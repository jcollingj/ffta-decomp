#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "content_payload_callback.h"
#include "content_tree_payload_callback.h"
#include "list_accessors.h"
#include "list_traversal.h"
#include "tree_reverse_visit.h"

#define NODE_COUNT 7

struct Fixture {
    struct Container owner;
    struct TreeCountNode nodes[NODE_COUNT];
    struct ContentPayloadCallbackPair pair;
};

struct CallbackState {
    u32 calls;
    void *expected_context;
    void *values[NODE_COUNT];
    void *contexts[NODE_COUNT];
    int results[NODE_COUNT];
    unsigned char guard[16];
};

static int payload_objects[4];

static const unsigned in_order[] = { 0, 1, 2, 3, 4, 5, 6 };
static const unsigned reverse_order[] = { 6, 5, 4, 3, 2, 1, 0 };
static const unsigned char left_child[] = { 255, 0, 255, 1, 255, 4, 255 };
static const unsigned char right_child[] = { 255, 2, 255, 5, 255, 6, 255 };
static const unsigned char parent[] = { 1, 3, 1, 255, 5, 3, 5 };
static void *const node_payloads[NODE_COUNT] = {
    &payload_objects[0], 0, &payload_objects[1], &payload_objects[2],
    &payload_objects[3], &payload_objects[0], &payload_objects[1]
};
static const u32 node_keys[NODE_COUNT] = {
    0x10203040, 0x55667788, 0x89ABCDEF, 0x01020304,
    0x76543210, 0xFFFFFFFF, 0x80000000
};

typedef char PairContextAtOffsetZero[
    offsetof(struct ContentPayloadCallbackPair, context) == 0 ? 1 : -1];
typedef char PairCallbackAtNativeWord[
    offsetof(struct ContentPayloadCallbackPair, callback) == sizeof(void *) ? 1 : -1];
typedef char PairNativeSize[
    sizeof(struct ContentPayloadCallbackPair) == 2 * sizeof(void *) ? 1 : -1];

static int record_payload(void *value, void *context)
{
    struct CallbackState *state = (struct CallbackState *)context;
    u32 index = state->calls;

    assert(context == state->expected_context);
    assert(index < NODE_COUNT);
    state->values[index] = value;
    state->contexts[index] = context;
    state->calls++;
    state->guard[index] ^= (unsigned char)(0x31 + index);
    return state->results[index];
}

static void initialize_fixture(struct Fixture *fixture,
                               struct CallbackState *state)
{
    unsigned i;

    memset(fixture, 0xA5, sizeof(*fixture));
    memset(state, 0xA5, sizeof(*state));
    fixture->owner.allocator_owner = 0;
    fixture->owner.list_head = 0;
    fixture->owner.tree_root = &fixture->nodes[3];
    for (i = 0; i < NODE_COUNT; i++) {
        struct TreeCountNode *node = &fixture->nodes[i];
        node->unknown00 = node_keys[i];
        node->unknown04 = (ListPayload)(unsigned long)node_payloads[i];
        node->parent = parent[i] == 255 ? 0 : &fixture->nodes[parent[i]];
        node->child_a = left_child[i] == 255 ? 0 :
            &fixture->nodes[left_child[i]];
        node->child_b = right_child[i] == 255 ? 0 :
            &fixture->nodes[right_child[i]];
        node->flags = 0xC0010000u + i;
    }
    state->calls = 0;
    state->expected_context = state;
    fixture->pair.context = state;
    fixture->pair.callback = record_payload;
}

static void initialize_expected_fixture(struct Fixture *expected,
                                        const struct Fixture *actual,
                                        struct CallbackState *state)
{
    unsigned i;

    *expected = *actual;
    expected->owner.tree_root = (struct TreeCountNode *)&actual->nodes[3];
    for (i = 0; i < NODE_COUNT; i++) {
        expected->nodes[i].parent = parent[i] == 255 ? 0 :
            (struct TreeCountNode *)&actual->nodes[parent[i]];
        expected->nodes[i].child_a = left_child[i] == 255 ? 0 :
            (struct TreeCountNode *)&actual->nodes[left_child[i]];
        expected->nodes[i].child_b = right_child[i] == 255 ? 0 :
            (struct TreeCountNode *)&actual->nodes[right_child[i]];
    }
    expected->pair.context = state;
}

static void initialize_expected_state(struct CallbackState *expected,
                                      const struct CallbackState *actual,
                                      const unsigned *order, unsigned calls,
                                      const int *results)
{
    unsigned i;

    *expected = *actual;
    expected->calls = calls;
    expected->expected_context = (void *)actual;
    for (i = 0; i < calls; i++) {
        expected->values[i] = node_payloads[order[i]];
        expected->contexts[i] = (void *)actual;
        expected->results[i] = results[i];
        expected->guard[i] = (unsigned char)(0xA5 ^ (0x31 + i));
    }
}

static void verify_traversal(int reverse, unsigned calls, const int *results,
                             int stop_at_zero)
{
    struct Fixture actual;
    struct Fixture expected;
    struct CallbackState state;
    struct CallbackState expected_state;
    const unsigned *order = reverse ? reverse_order : in_order;
    unsigned i;

    initialize_fixture(&actual, &state);
    for (i = 0; i < NODE_COUNT; i++)
        state.results[i] = results[i];
    initialize_expected_fixture(&expected, &actual, &state);
    initialize_expected_state(&expected_state, &state, order, calls, results);

    if (reverse)
        TraverseTreeReverseInOrder(&actual.owner,
                                   ContentTreePayloadCallbackAdapter,
                                   &actual.pair);
    else
        TraverseTreeInOrder(&actual.owner, ContentTreePayloadCallbackAdapter,
                            &actual.pair);

    assert(state.calls == calls);
    for (i = 0; i < calls; i++)
        assert(state.values[i] == node_payloads[order[i]]);
    if (stop_at_zero)
        assert(results[calls - 1] == 0);
    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
    assert(memcmp(&state, &expected_state, sizeof(state)) == 0);
}

static void test_tree_traversal_composition(void)
{
    static const int continue_results[NODE_COUNT] = { 1, 7, -1, 1, 7, -1, 1 };
    static const int stop_results[NODE_COUNT] = { 1, 7, 0, -1, 1, 7, -1 };

    verify_traversal(0, NODE_COUNT, continue_results, 0);
    verify_traversal(1, NODE_COUNT, continue_results, 0);
    verify_traversal(0, 3, stop_results, 1);
    verify_traversal(1, 3, stop_results, 1);
}

static void test_adapter_returns_callback_integer_unchanged(void)
{
    static const int results[] = { 0, 1, 7, -1 };
    unsigned i;

    for (i = 0; i < sizeof(results) / sizeof(results[0]); i++) {
        struct TreeCountNode node;
        struct ContentPayloadCallbackPair pair;
        struct CallbackState state;
        struct CallbackState expected_state;
        struct TreeCountNode expected_node;
        int result;

        memset(&node, 0xA5, sizeof(node));
        node.unknown00 = 0xA1B2C3D4u;
        node.unknown04 = (ListPayload)(unsigned long)node_payloads[i];
        expected_node = node;
        memset(&state, 0xA5, sizeof(state));
        state.calls = 0;
        state.expected_context = &state;
        state.results[0] = results[i];
        pair.context = &state;
        pair.callback = record_payload;
        initialize_expected_state(&expected_state, &state, in_order, 1,
                                  &results[i]);
        expected_state.values[0] = node_payloads[i];

        result = ContentTreePayloadCallbackAdapter(&node, &pair);

        assert(result == results[i]);
        assert(memcmp(&node, &expected_node, sizeof(node)) == 0);
        assert(memcmp(&state, &expected_state, sizeof(state)) == 0);
        assert(pair.context == &state);
        assert(pair.callback == record_payload);
    }
}

int main(void)
{
    test_tree_traversal_composition();
    test_adapter_returns_callback_integer_unchanged();
    return 0;
}
