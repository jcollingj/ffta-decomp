#include <assert.h>
#include <string.h>
#include "content_append.h"

struct Output {
    unsigned char before[8];
    void *values[3];
    unsigned char after[8];
};

struct Fixture {
    struct ContentAppendState state;
    struct Output output;
};

static void initialize(struct Fixture *fixture, u32 count)
{
    memset(fixture, 0xA5, sizeof(*fixture));
    fixture->state.count = count;
    fixture->state.cursor = &fixture->output.values[0];
}

static void test_successive_native_pointer_values(void)
{
    struct Fixture actual;
    struct Fixture expected;
    int payloads[3];

    initialize(&actual, 7);
    expected = actual;
    expected.output.values[0] = &payloads[0];
    expected.state.count = 8;
    expected.state.cursor = &actual.output.values[1];
    assert(ContentAppendCallback(&payloads[0], &actual.state) == 1);
    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);

    expected.output.values[1] = &payloads[1];
    expected.state.count = 9;
    expected.state.cursor = &actual.output.values[2];
    assert(ContentAppendCallback(&payloads[1], &actual.state) == 1);
    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);

    expected.output.values[2] = &payloads[2];
    expected.state.count = 10;
    expected.state.cursor = &actual.output.values[3];
    assert(ContentAppendCallback(&payloads[2], &actual.state) == 1);
    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
}

static void test_u32_counter_wrap(void)
{
    struct Fixture actual;
    struct Fixture expected;

    initialize(&actual, 0xffffffffu);
    expected = actual;
    expected.output.values[0] = 0;
    expected.state.count = 0;
    expected.state.cursor = &actual.output.values[1];
    assert(ContentAppendCallback(0, &actual.state) == 1);
    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
}

int main(void)
{
    test_successive_native_pointer_values();
    test_u32_counter_wrap();
    return 0;
}
