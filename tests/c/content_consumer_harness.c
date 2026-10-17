#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "container.h"
#include "content_consumer.h"
#include "content_fill.h"
#include "content_globals.h"
#include "content_record.h"
#include "content_selection_lookup.h"
#include "content_state.h"
#include "list_count.h"
#include "list_count_wrappers.h"

struct Fixture {
    struct ContentConsumerContext input;
    struct ListCountContext context;
    struct ContentConsumerEntry selection;
    struct ContentConsumerEntry entries[13];
    struct UnitFieldView objects[13];
    struct Container owner14;
    struct Container owner18;
    struct ListCountNode nodes14[3];
    struct ListCountNode nodes18[2];
    u16 first_output;
    u16 second_output;
};

struct StubState {
    struct Fixture *fixture;
    signed int list_count;
    u32 selection_present;
    unsigned trace[8];
    unsigned trace_count;
};

static struct StubState gStub;
struct ListCountContext *gContentConsumerContext;
struct ContentGlobalPointerOwner *gContentGlobalPointerOwner;
const struct ContentRecord gContentRecords[2] = {
    { {0}, 0, {0x39, 0, 0, 0, 0, 0, 0, 0} },
    { {0}, 0, {0xa7, 0, 0, 0, 0, 0, 0, 0} }
};
struct ContentIndexState gContentState __attribute__((aligned(4)));

static void trace(unsigned event)
{
    assert(gStub.trace_count < sizeof(gStub.trace) / sizeof(gStub.trace[0]));
    gStub.trace[gStub.trace_count++] = event;
}

int FillContentConsumerEntries(struct ListCountContext *context,
                               void **entries)
{
    unsigned i;
    assert(context == &gStub.fixture->context);
    trace(3);
    for (i = 0; i < 13; i++)
        entries[i] = (void *)&gStub.fixture->entries[i];
    return gStub.list_count;
}

struct ContentConsumerEntry *ResolveContentConsumerSelection(
    struct ListCountContext *context, const void *selector)
{
    assert(context == &gStub.fixture->context);
    assert(selector == &gStub.fixture->input);
    trace(4);
    return &gStub.fixture->selection;
}

u32 HasContentConsumerSelection(struct ListCountContext *context)
{
    assert(context == &gStub.fixture->context);
    trace(7);
    return gStub.selection_present;
}

static void reset_fixture(struct Fixture *fixture)
{
    unsigned i;
    memset(fixture, 0, sizeof(*fixture));
    memset(&gStub, 0, sizeof(gStub));
    memset(&gContentState, 0, sizeof(gContentState));
    fixture->first_output = 0xbeef;
    fixture->second_output = 0xdead;
    gStub.fixture = fixture;
    gContentConsumerContext = &fixture->context;
    fixture->context.unknown14 = &fixture->owner14;
    fixture->context.unknown18 = &fixture->owner18;
    for (i = 0; i < 3; i++) {
        fixture->nodes14[i].next = i == 2 ? 0 : &fixture->nodes14[i + 1];
    }
    for (i = 0; i < 2; i++) {
        fixture->nodes18[i].next = i == 1 ? 0 : &fixture->nodes18[i + 1];
    }
    fixture->owner14.list_head = &fixture->nodes14[0];
    fixture->owner18.list_head = &fixture->nodes18[0];
}

static void assert_full_fixture(const struct Fixture *fixture,
                                const struct Fixture *expected)
{
    assert(memcmp(fixture, expected, sizeof(*fixture)) == 0);
}

static void assert_trace(const unsigned *expected, unsigned count)
{
    assert(gStub.trace_count == count);
    assert(memcmp(gStub.trace, expected, count * sizeof(expected[0])) == 0);
}

static void test_content_list_counts(void)
{
    static const signed int counts[] = {-1, 0, 1, 12, 13, 14};
    static const u16 expected_qualifying[] = {0, 0, 0, 6, 6, 6};
    static const unsigned trace_true[] = {3};
    unsigned n;

    for (n = 0; n < sizeof(counts) / sizeof(counts[0]); n++) {
        struct Fixture fixture, expected;
        unsigned i;
        reset_fixture(&fixture);
        fixture.input.flags = 0x10;
        gStub.list_count = counts[n];
        gContentState.currentIndex = 1;
        for (i = 0; i < 13; i++) {
            fixture.entries[i].object = &fixture.objects[i];
            fixture.objects[i].half18[8] = (i & 1) ? 1 : 2;
        }
        if (counts[n] == 1)
            fixture.entries[0].object = 0;
        memcpy(&expected, &fixture, sizeof(expected));
        expected.first_output = expected_qualifying[n];
        expected.second_output = 0xa7;
        {
            struct ContentRecord expected_records[2];
            struct ContentIndexState expected_index_state = gContentState;
            memcpy(expected_records, gContentRecords, sizeof(expected_records));

            ConsumeContentRecord(&fixture.input, &fixture.input,
                                 &fixture.first_output, &fixture.second_output);
            assert_full_fixture(&fixture, &expected);
            assert(memcmp(gContentRecords, expected_records,
                          sizeof(expected_records)) == 0);
            assert(memcmp(&gContentState, &expected_index_state,
                          sizeof(expected_index_state)) == 0);
            assert_trace(trace_true, sizeof(trace_true) / sizeof(trace_true[0]));
        }
    }
}

static void test_content_order_and_flags(void)
{
    struct Fixture fixture, expected;
    static const unsigned trace_true[] = {3};
    unsigned i;

    reset_fixture(&fixture);
    fixture.input.flags = 0x10;
    gStub.list_count = 13;
    gContentState.currentIndex = 0;
    for (i = 0; i < 13; i++) {
        fixture.entries[i].object = &fixture.objects[i];
        fixture.objects[i].half18[8] =
            (i == 1 || i == 4 || i == 12) ? 1 : 0x8000;
    }
    fixture.entries[2].object = 0; /* Null is skipped; entry 4 still counts. */
    memcpy(&expected, &fixture, sizeof(expected));
    expected.first_output = 3;
    expected.second_output = 0x39;

    {
        struct ContentRecord expected_records[2];
        struct ContentIndexState expected_index_state = gContentState;
        memcpy(expected_records, gContentRecords, sizeof(expected_records));
        ConsumeContentRecord(&fixture.input, &fixture.input,
                             &fixture.first_output, &fixture.second_output);
        assert(memcmp(gContentRecords, expected_records,
                      sizeof(expected_records)) == 0);
        assert(memcmp(&gContentState, &expected_index_state,
                      sizeof(expected_index_state)) == 0);
    }
    assert_full_fixture(&fixture, &expected);
    assert_trace(trace_true, sizeof(trace_true) / sizeof(trace_true[0]));
}

static void test_alternate_branch_wrap_and_counts(void)
{
    struct Fixture fixture, expected;
    static const unsigned trace_false[] = {4, 7};

    reset_fixture(&fixture);
    fixture.input.flags = 0;
    fixture.selection.value5c = 0xffff;
    gStub.selection_present = 0x80000000;
    memcpy(&expected, &fixture, sizeof(expected));
    expected.first_output = 0;
    expected.second_output = 6; /* 3 + 2 + true. */

    ConsumeContentRecord(&fixture.input, &fixture.input,
                         &fixture.first_output, &fixture.second_output);
    assert_full_fixture(&fixture, &expected);
    assert_trace(trace_false, sizeof(trace_false) / sizeof(trace_false[0]));

    reset_fixture(&fixture);
    fixture.input.flags = 0;
    fixture.selection.value5c = 0x1234;
    gStub.selection_present = 0;
    fixture.owner14.list_head = 0;
    fixture.owner18.list_head = 0;
    memcpy(&expected, &fixture, sizeof(expected));
    expected.first_output = 0x1235;
    expected.second_output = 0;

    ConsumeContentRecord(&fixture.input, &fixture.input,
                         &fixture.first_output, &fixture.second_output);
    assert_full_fixture(&fixture, &expected);
    assert_trace(trace_false, sizeof(trace_false) / sizeof(trace_false[0]));
}

int main(void)
{
    if (sizeof(void *) == 4) {
        assert(offsetof(struct ContentRecord, values) == 28);
        assert(offsetof(struct ContentRecord, unknown20) == 32);
        assert(sizeof(struct ContentRecord) == 40);
    } else {
        assert(offsetof(struct ContentRecord, values) == 32);
        assert(offsetof(struct ContentRecord, unknown20) == 40);
        assert(sizeof(struct ContentRecord) == 48);
    }
    assert(offsetof(struct UnitFieldView, half18) + 8 * sizeof(u16) == 40);
    assert(offsetof(struct ContentConsumerContext, flags) == 16);
    test_content_list_counts();
    test_content_order_and_flags();
    test_alternate_branch_wrap_and_counts();
    return 0;
}
