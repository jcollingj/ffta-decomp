#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "content_append.h"
#include "content_payload_callback.h"
#include "list_accessors.h"
#include "list_visit.h"

#define NODE_COUNT 3

struct Output {
    unsigned char before[8];
    void *values[NODE_COUNT];
    unsigned char after[8];
};

struct Fixture {
    struct Container owner;
    struct ListCountNode nodes[NODE_COUNT];
    struct ContentAppendState append_state;
    struct Output output;
    struct ContentPayloadCallbackPair pair;
};

struct ControlledCallbackState {
    u32 calls;
    int results[NODE_COUNT];
    void *values[NODE_COUNT];
    unsigned char guard[8];
};

static int payloads[NODE_COUNT];

typedef char PairContextAtOffsetZero[
    offsetof(struct ContentPayloadCallbackPair, context) == 0 ? 1 : -1];
typedef char PairCallbackAtNativeWord[
    offsetof(struct ContentPayloadCallbackPair, callback) == sizeof(void *) ? 1 : -1];
typedef char PairNativeSize[
    sizeof(struct ContentPayloadCallbackPair) == 2 * sizeof(void *) ? 1 : -1];

static void initialize_append_fixture(struct Fixture *fixture)
{
    unsigned i;

    memset(fixture, 0xA5, sizeof(*fixture));
    fixture->owner.allocator_owner = 0;
    fixture->owner.list_head = &fixture->nodes[0];
    fixture->owner.tree_root = 0;
    for (i = 0; i < NODE_COUNT; i++) {
        fixture->nodes[i].unknown00 = (ListPayload)(unsigned long)&payloads[i];
        fixture->nodes[i].previous = i == 0 ? 0 : &fixture->nodes[i - 1];
        fixture->nodes[i].next = i + 1 == NODE_COUNT ? 0 : &fixture->nodes[i + 1];
    }
    fixture->append_state.count = 0;
    fixture->append_state.cursor = &fixture->output.values[0];
    fixture->pair.context = &fixture->append_state;
    fixture->pair.callback = ContentAppendCallback;
}

static void point_expected_image_at_actual(struct Fixture *expected,
                                           const struct Fixture *actual)
{
    unsigned i;

    expected->owner.list_head = (struct ListCountNode *)&actual->nodes[0];
    for (i = 0; i < NODE_COUNT; i++) {
        expected->nodes[i].previous = i == 0 ? 0 :
            (struct ListCountNode *)&actual->nodes[i - 1];
        expected->nodes[i].next = i + 1 == NODE_COUNT ? 0 :
            (struct ListCountNode *)&actual->nodes[i + 1];
    }
    expected->append_state.cursor = (void **)&actual->output.values[0];
    expected->pair.context = (void *)&actual->append_state;
}

static void test_three_node_append_composition(void)
{
    struct Fixture actual;
    struct Fixture expected;

    initialize_append_fixture(&actual);
    initialize_append_fixture(&expected);
    point_expected_image_at_actual(&expected, &actual);
    expected.output.values[0] = &payloads[0];
    expected.output.values[1] = &payloads[1];
    expected.output.values[2] = &payloads[2];
    expected.append_state.count = NODE_COUNT;
    expected.append_state.cursor = (void **)&actual.output.values[NODE_COUNT];

    VisitLinkedNodes(&actual.owner, ContentPayloadCallbackAdapter, &actual.pair);

    assert(actual.output.values[0] == &payloads[0]);
    assert(actual.output.values[1] == &payloads[1]);
    assert(actual.output.values[2] == &payloads[2]);
    assert(actual.pair.context == &actual.append_state);
    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
}

static void test_empty_list_is_unchanged(void)
{
    struct Fixture actual;
    struct Fixture expected;

    initialize_append_fixture(&actual);
    initialize_append_fixture(&expected);
    point_expected_image_at_actual(&expected, &actual);
    actual.owner.list_head = 0;
    expected.owner.list_head = 0;

    VisitLinkedNodes(&actual.owner, ContentPayloadCallbackAdapter, &actual.pair);

    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
}

static int controlled_callback(void *value, void *context)
{
    struct ControlledCallbackState *state =
        (struct ControlledCallbackState *)context;
    u32 index = state->calls;

    state->values[index] = value;
    state->calls++;
    state->guard[index] ^= 0x5A;
    return state->results[index];
}

static void test_callback_arguments_and_full_integer_return(void)
{
    static const int results[] = { 0, 1, 7, -1 };
    unsigned i;

    for (i = 0; i < sizeof(results) / sizeof(results[0]); i++) {
        struct ListCountNode node;
        struct ContentPayloadCallbackPair pair;
        struct ControlledCallbackState state;
        struct ControlledCallbackState expected_state;
        struct ListCountNode expected_node;
        u32 result;

        memset(&node, 0xA5, sizeof(node));
        node.unknown00 = (ListPayload)(unsigned long)&payloads[i % NODE_COUNT];
        memset(&expected_node, 0xA5, sizeof(expected_node));
        expected_node.unknown00 = node.unknown00;
        memset(&state, 0xA5, sizeof(state));
        state.calls = 0;
        state.results[0] = results[i];
        state.values[0] = 0;
        memset(&expected_state, 0xA5, sizeof(expected_state));
        expected_state.calls = 1;
        expected_state.results[0] = results[i];
        expected_state.values[0] = &payloads[i % NODE_COUNT];
        expected_state.guard[0] ^= 0x5A;
        pair.context = &state;
        pair.callback = controlled_callback;

        result = ContentPayloadCallbackAdapter(&node, &pair);

        assert(result == (u32)results[i]);
        assert(memcmp(&node, &expected_node, sizeof(node)) == 0);
        assert(memcmp(&state, &expected_state, sizeof(state)) == 0);
        assert(pair.context == &state);
        assert(pair.callback == controlled_callback);
    }
}

static void test_visit_stops_on_zero_and_continues_on_other_values(void)
{
    struct Fixture actual;
    struct Fixture expected;
    struct ControlledCallbackState state;
    struct ControlledCallbackState expected_state;

    initialize_append_fixture(&actual);
    initialize_append_fixture(&expected);
    point_expected_image_at_actual(&expected, &actual);
    memset(&state, 0xA5, sizeof(state));
    memset(&expected_state, 0xA5, sizeof(expected_state));
    state.calls = 0;
    state.results[0] = 7;
    state.results[1] = 0;
    state.results[2] = -1;
    state.values[0] = 0;
    state.values[1] = 0;
    expected_state.calls = 2;
    expected_state.results[0] = 7;
    expected_state.results[1] = 0;
    expected_state.results[2] = -1;
    expected_state.values[0] = &payloads[0];
    expected_state.values[1] = &payloads[1];
    expected_state.guard[0] ^= 0x5A;
    expected_state.guard[1] ^= 0x5A;
    actual.pair.context = &state;
    actual.pair.callback = controlled_callback;
    expected.pair.context = &state;
    expected.pair.callback = controlled_callback;

    VisitLinkedNodes(&actual.owner, ContentPayloadCallbackAdapter, &actual.pair);

    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
    assert(memcmp(&state, &expected_state, sizeof(state)) == 0);
}

static void test_negative_and_nonunit_returns_continue(void)
{
    struct Fixture actual;
    struct Fixture expected;
    struct ControlledCallbackState state;
    struct ControlledCallbackState expected_state;

    initialize_append_fixture(&actual);
    initialize_append_fixture(&expected);
    point_expected_image_at_actual(&expected, &actual);
    memset(&state, 0xA5, sizeof(state));
    memset(&expected_state, 0xA5, sizeof(expected_state));
    state.calls = 0;
    state.results[0] = -1;
    state.results[1] = 7;
    state.results[2] = 1;
    state.values[0] = 0;
    state.values[1] = 0;
    state.values[2] = 0;
    expected_state.calls = NODE_COUNT;
    expected_state.results[0] = -1;
    expected_state.results[1] = 7;
    expected_state.results[2] = 1;
    expected_state.values[0] = &payloads[0];
    expected_state.values[1] = &payloads[1];
    expected_state.values[2] = &payloads[2];
    expected_state.guard[0] ^= 0x5A;
    expected_state.guard[1] ^= 0x5A;
    expected_state.guard[2] ^= 0x5A;
    actual.pair.context = &state;
    actual.pair.callback = controlled_callback;
    expected.pair.context = &state;
    expected.pair.callback = controlled_callback;

    VisitLinkedNodes(&actual.owner, ContentPayloadCallbackAdapter, &actual.pair);

    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
    assert(memcmp(&state, &expected_state, sizeof(state)) == 0);
}

int main(void)
{
    test_three_node_append_composition();
    test_empty_list_is_unchanged();
    test_callback_arguments_and_full_integer_return();
    test_visit_stops_on_zero_and_continues_on_other_values();
    test_negative_and_nonunit_returns_continue();
    return 0;
}
