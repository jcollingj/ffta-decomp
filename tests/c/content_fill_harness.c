#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "container.h"
#include "content_fill.h"
#include "list_count.h"

struct Fixture {
    struct ListCountContext context;
    struct Container owner10;
    struct Container owner14;
    struct Container owner18;
    struct ListCountNode nodes10[1];
    struct ListCountNode nodes14[13];
    struct ListCountNode nodes18[2];
    int payloads10[1];
    int payloads14[13];
    int payloads18[2];
};

struct GuardedOutput {
    uintptr_t before;
    void *values[13];
    uintptr_t after;
};

static int sentinel_a;
static int sentinel_b;

static void link_nodes(struct ListCountNode *nodes, int *payloads,
                       unsigned count)
{
    unsigned i;

    for (i = 0; i < count; i++) {
        nodes[i].unknown00 = (ListPayload)(unsigned long)&payloads[i];
        nodes[i].previous = i == 0 ? 0 : &nodes[i - 1];
        nodes[i].next = i + 1 == count ? 0 : &nodes[i + 1];
    }
}

static void initialize(struct Fixture *fixture, struct GuardedOutput *output,
                       unsigned count)
{
    unsigned i;

    memset(fixture, 0, sizeof(*fixture));
    memset(output, 0, sizeof(*output));
    fixture->context.unknown10 = &fixture->owner10;
    fixture->context.unknown14 = &fixture->owner14;
    fixture->context.unknown18 = &fixture->owner18;
    link_nodes(fixture->nodes10, fixture->payloads10, 1);
    link_nodes(fixture->nodes14, fixture->payloads14, count);
    link_nodes(fixture->nodes18, fixture->payloads18, 2);
    fixture->owner10.list_head = &fixture->nodes10[0];
    fixture->owner14.list_head = count == 0 ? 0 : &fixture->nodes14[0];
    fixture->owner18.list_head = &fixture->nodes18[0];
    output->before = (uintptr_t)0x13579bdf;
    output->after = (uintptr_t)0x2468ace0;
    for (i = 0; i < 13; i++)
        output->values[i] = (i & 1) ? (void *)&sentinel_a : (void *)&sentinel_b;
}

static void check_case(unsigned count)
{
    struct Fixture actual, expected_fixture;
    struct GuardedOutput output, expected_output;
    unsigned i;
    int result;

    initialize(&actual, &output, count);
    memcpy(&expected_fixture, &actual, sizeof(expected_fixture));
    memcpy(&expected_output, &output, sizeof(expected_output));
    for (i = 0; i < count; i++)
        expected_output.values[i] = &actual.payloads14[i];

    result = FillContentConsumerEntries(&actual.context, output.values);

    assert(result == (int)count);
    assert(memcmp(&actual, &expected_fixture, sizeof(actual)) == 0);
    assert(memcmp(&output, &expected_output, sizeof(output)) == 0);
}

int main(void)
{
    assert(offsetof(struct ListCountContext, unknown10) == 16);
    assert(offsetof(struct ListCountContext, unknown14) == 16 + sizeof(void *));
    assert(offsetof(struct ListCountContext, unknown18) == 16 + 2 * sizeof(void *));
    check_case(0);
    check_case(1);
    check_case(3);
    check_case(13);
    return 0;
}
