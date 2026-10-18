#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "content_tree_context_visit.h"
#include "list_accessors.h"
#include "list_traversal.h"
#include "tree_reverse_visit.h"

#define OWNER_COUNT 3
#define NODE_COUNT 3

typedef void (*ContextTreeVisitor)(const struct ListCountContext *,
                                   ContentPayloadCallback, void *);

struct CallbackState {
    int calls;
    void *values[NODE_COUNT];
    int returns[NODE_COUNT];
    u32 effects[NODE_COUNT];
    u32 untouched[2];
};

struct Fixture {
    struct ListCountContext context;
    struct Container owners[OWNER_COUNT];
    struct TreeCountNode nodes[OWNER_COUNT][NODE_COUNT];
    struct CallbackState callback;
};

static u32 payload_tokens[OWNER_COUNT][NODE_COUNT];

static const ContextTreeVisitor visitors[6] = {
    VisitContextTreePayloadsInOrderAt10,
    VisitContextTreePayloadsInOrderAt14,
    VisitContextTreePayloadsInOrderAt18,
    VisitContextTreePayloadsReverseAt10,
    VisitContextTreePayloadsReverseAt14,
    VisitContextTreePayloadsReverseAt18
};

/* Fixed node indices are independently derived from the three-node fixtures:
   in order is left/root/right; reverse order is right/root/left. */
static const unsigned expected_node_order[6][NODE_COUNT] = {
    { 0, 1, 2 }, { 0, 1, 2 }, { 0, 1, 2 },
    { 2, 1, 0 }, { 2, 1, 0 }, { 2, 1, 0 }
};

static void initialize_fixture(struct Fixture *fixture)
{
    unsigned owner;
    unsigned node;

    memset(fixture, 0xA5, sizeof(*fixture));
    memset(payload_tokens, 0, sizeof(payload_tokens));
    memset(&fixture->callback, 0, sizeof(fixture->callback));
    memset(&fixture->context, 0x5A, sizeof(fixture->context));

    for (owner = 0; owner < OWNER_COUNT; owner++) {
        fixture->owners[owner].allocator_owner =
            (void *)(unsigned long)(0x1100 + owner * 0x100);
        fixture->owners[owner].list_head =
            (struct ListCountNode *)(unsigned long)(0x2200 + owner * 0x100);
        fixture->owners[owner].tree_root = &fixture->nodes[owner][1];
        for (node = 0; node < NODE_COUNT; node++) {
            struct TreeCountNode *entry = &fixture->nodes[owner][node];

            entry->unknown00 = 0xA5000000u + owner * 0x100u + node;
            entry->unknown04 = (ListPayload)(unsigned long)
                &payload_tokens[owner][node];
            entry->parent = node == 1 ? 0 : &fixture->nodes[owner][1];
            entry->child_a = node == 1 ? &fixture->nodes[owner][0] : 0;
            entry->child_b = node == 1 ? &fixture->nodes[owner][2] : 0;
            entry->flags = 0x5A000000u + owner * 0x100u + node;
        }
    }
    fixture->context.unknown10 = &fixture->owners[0];
    fixture->context.unknown14 = &fixture->owners[1];
    fixture->context.unknown18 = &fixture->owners[2];
    fixture->callback.untouched[0] = 0xCAFEBABEu;
    fixture->callback.untouched[1] = 0x12345678u;
}

static void set_expected_callback(struct CallbackState *expected,
                                  unsigned visitor, unsigned owner,
                                  unsigned count)
{
    unsigned i;

    expected->calls = (int)count;
    for (i = 0; i < count; i++) {
        unsigned node = expected_node_order[visitor][i];
        expected->values[i] = &payload_tokens[owner][node];
        expected->effects[i] = 0xB6000000u + (u32)node;
    }
}

static int record_payload(void *payload, void *context)
{
    struct CallbackState *state = (struct CallbackState *)context;
    int slot = state->calls;
    unsigned owner;
    unsigned node;
    unsigned found = NODE_COUNT;

    assert(slot >= 0 && slot < NODE_COUNT);
    for (owner = 0; owner < OWNER_COUNT; owner++) {
        for (node = 0; node < NODE_COUNT; node++) {
            if (payload == &payload_tokens[owner][node])
                found = node;
        }
    }
    assert(found < NODE_COUNT);
    state->values[slot] = payload;
    state->effects[slot] = 0xB6000000u + (u32)found;
    state->calls++;
    return state->returns[slot];
}

static void run_case(unsigned visitor, unsigned mode)
{
    struct Fixture actual;
    struct Fixture expected;
    unsigned owner = visitor % OWNER_COUNT;
    unsigned count;
    unsigned i;

    initialize_fixture(&actual);
    memcpy(&expected, &actual, sizeof(expected));

    if (mode == 0) {
        actual.owners[owner].tree_root = 0;
        expected.owners[owner].tree_root = 0;
        count = 0;
    } else if (mode == 1) {
        static const int negative_continuation[NODE_COUNT] = { -1, -2, -3 };
        count = NODE_COUNT;
        for (i = 0; i < NODE_COUNT; i++) {
            actual.callback.returns[i] = negative_continuation[i];
            expected.callback.returns[i] = negative_continuation[i];
        }
    } else {
        static const int stop_after_second[NODE_COUNT] = { 1, 0, 7 };
        count = 2;
        for (i = 0; i < NODE_COUNT; i++) {
            actual.callback.returns[i] = stop_after_second[i];
            expected.callback.returns[i] = stop_after_second[i];
        }
    }

    set_expected_callback(&expected.callback, visitor, owner, count);
    visitors[visitor](&actual.context, record_payload, &actual.callback);

    assert(actual.callback.calls == (int)count);
    for (i = 0; i < count; i++) {
        assert(actual.callback.values[i] ==
               &payload_tokens[owner][expected_node_order[visitor][i]]);
    }
    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
}

static void test_all_six_context_tree_visitors(void)
{
    unsigned visitor;
    unsigned mode;

    for (visitor = 0; visitor < 6; visitor++) {
        for (mode = 0; mode < 3; mode++)
            run_case(visitor, mode);
    }
}

int main(void)
{
    test_all_six_context_tree_visitors();
    return 0;
}

