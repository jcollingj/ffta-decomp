#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "content_context_visit.h"
#include "list_accessors.h"
#include "list_visit.h"

#define SLOT_COUNT 3
#define NODE_COUNT 3

struct CallbackState {
    const struct ListCountContext *context;
    unsigned slot;
    unsigned calls;
    void *seen[NODE_COUNT];
    struct Container *observed_owner[NODE_COUNT];
    int results[NODE_COUNT];
    unsigned char guard[8];
};

struct Fixture {
    struct ListCountContext context;
    struct Container owners[SLOT_COUNT];
    struct ListCountNode nodes[SLOT_COUNT][NODE_COUNT];
    int payloads[SLOT_COUNT][NODE_COUNT];
    struct CallbackState callback;
};

typedef char ContextField10AtTargetOffset[
    sizeof(void *) != 4 || offsetof(struct ListCountContext, unknown10) == 0x10 ? 1 : -1];
typedef char ContextField14AtTargetOffset[
    sizeof(void *) != 4 || offsetof(struct ListCountContext, unknown14) == 0x14 ? 1 : -1];
typedef char ContextField18AtTargetOffset[
    sizeof(void *) != 4 || offsetof(struct ListCountContext, unknown18) == 0x18 ? 1 : -1];
typedef char PairHasNativeEightByteTargetLayout[
    sizeof(void *) != 4 || sizeof(struct ContentPayloadCallbackPair) == 8 ? 1 : -1];

static void initialize_fixture(struct Fixture *fixture)
{
    static const unsigned char prefix[16] = {
        0x10, 0x21, 0x32, 0x43, 0x54, 0x65, 0x76, 0x87,
        0x98, 0xA9, 0xBA, 0xCB, 0xDC, 0xED, 0xFE, 0x0F
    };
    unsigned slot;
    unsigned index;

    memset(fixture, 0xA5, sizeof(*fixture));
    memcpy(fixture->context.unknown00, prefix, sizeof(prefix));
    fixture->context.unknown10 = &fixture->owners[0];
    fixture->context.unknown14 = &fixture->owners[1];
    fixture->context.unknown18 = &fixture->owners[2];

    for (slot = 0; slot < SLOT_COUNT; slot++) {
        fixture->owners[slot].allocator_owner = &fixture->payloads[slot][0];
        fixture->owners[slot].list_head = &fixture->nodes[slot][0];
        fixture->owners[slot].tree_root = 0;
        for (index = 0; index < NODE_COUNT; index++) {
            fixture->payloads[slot][index] = (int)(100 * slot + index + 1);
            fixture->nodes[slot][index].unknown00 =
                (ListPayload)(unsigned long)&fixture->payloads[slot][index];
            fixture->nodes[slot][index].previous =
                index == 0 ? 0 : &fixture->nodes[slot][index - 1];
            fixture->nodes[slot][index].next =
                index + 1 == NODE_COUNT ? 0 : &fixture->nodes[slot][index + 1];
        }
    }

    fixture->callback.context = &fixture->context;
    fixture->callback.slot = 0;
    fixture->callback.calls = 0;
    for (index = 0; index < NODE_COUNT; index++) {
        fixture->callback.seen[index] = 0;
        fixture->callback.observed_owner[index] = 0;
        fixture->callback.results[index] = 0;
    }
    memset(fixture->callback.guard, 0xC3, sizeof(fixture->callback.guard));
}

static void rebase_expected(struct Fixture *expected,
                            const struct Fixture *actual)
{
    unsigned slot;
    unsigned index;

    expected->context.unknown10 = (struct Container *)&actual->owners[0];
    expected->context.unknown14 = (struct Container *)&actual->owners[1];
    expected->context.unknown18 = (struct Container *)&actual->owners[2];
    expected->callback.context = &actual->context;

    for (slot = 0; slot < SLOT_COUNT; slot++) {
        expected->owners[slot].allocator_owner =
            (void *)&actual->payloads[slot][0];
        expected->owners[slot].list_head =
            (struct ListCountNode *)&actual->nodes[slot][0];
        for (index = 0; index < NODE_COUNT; index++) {
            expected->nodes[slot][index].unknown00 =
                (ListPayload)(unsigned long)&actual->payloads[slot][index];
            expected->nodes[slot][index].previous = index == 0 ? 0 :
                (struct ListCountNode *)&actual->nodes[slot][index - 1];
            expected->nodes[slot][index].next = index + 1 == NODE_COUNT ? 0 :
                (struct ListCountNode *)&actual->nodes[slot][index + 1];
        }
    }
}

static struct Container *context_owner(const struct ListCountContext *context,
                                       unsigned slot)
{
    if (slot == 0)
        return context->unknown10;
    if (slot == 1)
        return context->unknown14;
    assert(slot == 2);
    return context->unknown18;
}

static int record_payload(void *payload, void *context)
{
    struct CallbackState *state = (struct CallbackState *)context;
    unsigned index = state->calls;

    assert(index < NODE_COUNT);
    state->seen[index] = payload;
    state->observed_owner[index] = context_owner(state->context, state->slot);
    state->guard[index] ^= 0x5A;
    state->calls++;
    return state->results[index];
}

static void configure_length(struct Fixture *fixture, unsigned slot,
                             unsigned length)
{
    assert(slot < SLOT_COUNT);
    assert(length <= NODE_COUNT);
    fixture->owners[slot].list_head =
        length == 0 ? 0 : &fixture->nodes[slot][0];
    if (length < NODE_COUNT)
        fixture->nodes[slot][length == 0 ? 0 : length - 1].next = 0;
}

static void configure_expected_length(struct Fixture *expected,
                                      const struct Fixture *actual,
                                      unsigned slot, unsigned length)
{
    expected->owners[slot].list_head = length == 0 ? 0 :
        (struct ListCountNode *)&actual->nodes[slot][0];
    if (length < NODE_COUNT)
        expected->nodes[slot][length == 0 ? 0 : length - 1].next = 0;
}

static void invoke(unsigned slot, unsigned length,
                   const int results[NODE_COUNT], unsigned expected_calls)
{
    struct Fixture actual;
    struct Fixture expected;
    unsigned index;

    initialize_fixture(&actual);
    initialize_fixture(&expected);
    configure_length(&actual, slot, length);
    rebase_expected(&expected, &actual);
    configure_expected_length(&expected, &actual, slot, length);

    actual.callback.slot = slot;
    expected.callback.slot = slot;
    for (index = 0; index < NODE_COUNT; index++) {
        actual.callback.results[index] = results[index];
        expected.callback.results[index] = results[index];
    }

    if (slot == 0)
        VisitContextPayloadsAt10(&actual.context, record_payload,
                                 &actual.callback);
    else if (slot == 1)
        VisitContextPayloadsAt14(&actual.context, record_payload,
                                 &actual.callback);
    else {
        assert(slot == 2);
        VisitContextPayloadsAt18(&actual.context, record_payload,
                                 &actual.callback);
    }

    expected.callback.calls = expected_calls;
    for (index = 0; index < expected_calls; index++) {
        expected.callback.seen[index] =
            (void *)&actual.payloads[slot][index];
        expected.callback.observed_owner[index] =
            (struct Container *)&actual.owners[slot];
        expected.callback.guard[index] ^= 0x5A;
    }

    assert(actual.callback.calls == expected_calls);
    for (index = 0; index < expected_calls; index++) {
        assert(actual.callback.seen[index] ==
               (void *)&actual.payloads[slot][index]);
        assert(actual.callback.observed_owner[index] == &actual.owners[slot]);
    }
    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
}

int main(void)
{
    static const int continue_all[NODE_COUNT] = { 7, 7, 7 };
    static const int stop_after_second[NODE_COUNT] = { 7, 0, 7 };
    static const int stop_after_first[NODE_COUNT] = { 0, 7, 7 };
    static const int unused[NODE_COUNT] = { 0, 0, 0 };
    unsigned slot;

    for (slot = 0; slot < SLOT_COUNT; slot++) {
        invoke(slot, 0, unused, 0);
        invoke(slot, 1, stop_after_first, 1);
        invoke(slot, NODE_COUNT, stop_after_second, 2);
        invoke(slot, NODE_COUNT, continue_all, NODE_COUNT);
    }
    return 0;
}
