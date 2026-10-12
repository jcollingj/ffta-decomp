#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "../../include/list_visit.h"
#include "../../src/list_visit.c"

struct Fixture {
    struct Container owner;
    struct ListCountNode nodes[6];
    struct ListCountNode guards[2];
};

struct Trace {
    struct ListCountNode *visited[8];
    unsigned count;
    unsigned stop_at;
    u32 stop_value;
    int replace_next;
    struct ListCountNode *replacement;
};

static u32 record_visit(struct ListCountNode *node, void *opaque)
{
    struct Trace *trace = (struct Trace *)opaque;
    unsigned position = trace->count++;

    assert(position < 8);
    trace->visited[position] = node;
    if (trace->replace_next && position == 0)
        node->next = trace->replacement;
    if (position == trace->stop_at)
        return trace->stop_value;
    return 1;
}

static void make_fixture(struct Fixture *fixture, unsigned length)
{
    unsigned i;

    memset(fixture, 0xA5, sizeof(*fixture));
    fixture->owner.allocator_owner = (void *)0xD00DFEED;
    fixture->owner.list_head = length == 0 ? 0 : &fixture->nodes[0];
    for (i = 0; i < 6; i++) {
        fixture->nodes[i].unknown00 = 0x11110000 + i;
        fixture->nodes[i].unknown04 = 0x22220000 + i;
        fixture->nodes[i].next = i + 1 < length ? &fixture->nodes[i + 1] : 0;
    }
    fixture->guards[0].unknown00 = 0x33334444;
    fixture->guards[0].unknown04 = 0x55556666;
    fixture->guards[0].next = &fixture->guards[1];
    fixture->guards[1].unknown00 = 0x77778888;
    fixture->guards[1].unknown04 = 0x9999AAAA;
    fixture->guards[1].next = 0;
}

static void check_trace(const struct Trace *trace,
                        struct ListCountNode *const *expected,
                        unsigned expected_count)
{
    unsigned i;
    assert(trace->count == expected_count);
    for (i = 0; i < expected_count; i++)
        assert(trace->visited[i] == expected[i]);
}

int main(void)
{
    struct Fixture fixture;
    struct Fixture before;
    struct Fixture expected_fixture;
    struct Trace trace;
    struct ListCountNode *expected[6];
    unsigned i;

    assert(offsetof(struct Container, list_head) ==
           (sizeof(void *) == 8 ? 8 : 4));
    assert(offsetof(struct ListCountNode, next) == 8);

    make_fixture(&fixture, 0);
    before = fixture;
    memset(&trace, 0, sizeof(trace));
    trace.stop_at = 99;
    VisitLinkedNodes(&fixture.owner, record_visit, &trace);
    check_trace(&trace, expected, 0);
    assert(memcmp(&fixture, &before, sizeof(fixture)) == 0);

    make_fixture(&fixture, 5);
    before = fixture;
    for (i = 0; i < 5; i++)
        expected[i] = &fixture.nodes[i];
    memset(&trace, 0, sizeof(trace));
    trace.stop_at = 99;
    VisitLinkedNodes(&fixture.owner, record_visit, &trace);
    check_trace(&trace, expected, 5);
    assert(memcmp(&fixture, &before, sizeof(fixture)) == 0);

    /* A zero result stops at every possible visit position. */
    for (i = 0; i < 5; i++) {
        make_fixture(&fixture, 5);
        before = fixture;
        memset(&trace, 0, sizeof(trace));
        trace.stop_at = i;
        trace.stop_value = 0;
        VisitLinkedNodes(&fixture.owner, record_visit, &trace);
        check_trace(&trace, expected, i + 1);
        assert(memcmp(&fixture, &before, sizeof(fixture)) == 0);
    }

    /* Any nonzero result continues, including values other than one. */
    make_fixture(&fixture, 5);
    before = fixture;
    memset(&trace, 0, sizeof(trace));
    trace.stop_at = 0;
    trace.stop_value = 2;
    VisitLinkedNodes(&fixture.owner, record_visit, &trace);
    check_trace(&trace, expected, 5);
    assert(memcmp(&fixture, &before, sizeof(fixture)) == 0);

    /* The next link is read after the callback and can be redirected. */
    make_fixture(&fixture, 5);
    expected_fixture = fixture;
    expected_fixture.nodes[0].next = &fixture.nodes[3];
    memset(&trace, 0, sizeof(trace));
    trace.stop_at = 99;
    trace.replace_next = 1;
    trace.replacement = &fixture.nodes[3];
    expected[0] = &fixture.nodes[0];
    expected[1] = &fixture.nodes[3];
    expected[2] = &fixture.nodes[4];
    VisitLinkedNodes(&fixture.owner, record_visit, &trace);
    check_trace(&trace, expected, 3);
    /* The expected fixture independently describes the single callback write. */
    assert(memcmp(&fixture, &expected_fixture, sizeof(fixture)) == 0);
    return 0;
}
