#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "../../include/list_circular_visit.h"
#include "../../src/nonmatching/list_circular_visit.c"

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
    struct Fixture *fixture;
    int redirect_next;
    struct ListCountNode *next_replacement;
    int replace_head;
    unsigned mutate_at;
    struct ListCountNode *head_replacement;
};

static u32 record_visit(struct ListCountNode *node, void *opaque)
{
    struct Trace *trace = (struct Trace *)opaque;
    unsigned position = trace->count++;

    assert(position < 8);
    trace->visited[position] = node;
    if (trace->redirect_next && position == 0)
        node->next = trace->next_replacement;
    if (trace->replace_head && position == trace->mutate_at)
        trace->fixture->owner.list_head = trace->head_replacement;
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
        fixture->nodes[i].previous = i > 0 && i < length ?
                                     &fixture->nodes[i - 1] : 0;
        fixture->nodes[i].next = i + 1 < length ? &fixture->nodes[i + 1] : 0;
    }
    fixture->guards[0].unknown00 = 0x33334444;
    fixture->guards[0].previous = 0;
    fixture->guards[0].next = &fixture->guards[1];
    fixture->guards[1].unknown00 = 0x77778888;
    fixture->guards[1].previous = &fixture->guards[0];
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

static void check_start(unsigned length, unsigned start_index)
{
    struct Fixture fixture;
    struct Fixture before;
    struct Trace trace;
    struct ListCountNode *expected[6];
    unsigned i;

    make_fixture(&fixture, length);
    before = fixture;
    memset(&trace, 0, sizeof(trace));
    trace.fixture = &fixture;
    trace.stop_at = 99;
    for (i = 0; i < length; i++)
        expected[i] = &fixture.nodes[(start_index + i) % length];
    VisitCircularListFrom(&fixture.owner, &fixture.nodes[start_index],
                          record_visit, &trace);
    check_trace(&trace, expected, length);
    assert(memcmp(&fixture, &before, sizeof(fixture)) == 0);
}

int main(void)
{
    struct Fixture fixture;
    struct Fixture before;
    struct Fixture expected_fixture;
    struct Trace trace;
    struct ListCountNode *expected[6];
    unsigned length;
    unsigned start;
    unsigned stop;

    assert(offsetof(struct Container, list_head) ==
           (sizeof(void *) == 8 ? 8 : 4));
    assert(offsetof(struct ListCountNode, previous) ==
           (sizeof(void *) == 8 ? 8 : 4));
    assert(offsetof(struct ListCountNode, next) ==
           (sizeof(void *) == 8 ? 16 : 8));

    /* Head, middle, and tail starts for one, three, and six nodes. */
    check_start(1, 0);
    check_start(3, 0);
    check_start(3, 1);
    check_start(3, 2);
    check_start(6, 0);
    check_start(6, 3);
    check_start(6, 5);

    /* A zero callback result stops at every point of each fixed trace. */
    for (length = 1; length <= 6; length++) {
        for (start = 0; start < length; start++) {
            for (stop = 0; stop < length; stop++) {
                unsigned count = 0;
                unsigned i;
                make_fixture(&fixture, length);
                before = fixture;
                memset(&trace, 0, sizeof(trace));
                trace.fixture = &fixture;
                trace.stop_at = stop;
                trace.stop_value = 0;
                for (i = 0; i <= stop; i++)
                    expected[count++] = &fixture.nodes[(start + i) % length];
                VisitCircularListFrom(&fixture.owner, &fixture.nodes[start],
                                      record_visit, &trace);
                check_trace(&trace, expected, count);
                assert(memcmp(&fixture, &before, sizeof(fixture)) == 0);
            }
        }
    }

    /* Any nonzero callback result continues, including a value other than one. */
    make_fixture(&fixture, 3);
    before = fixture;
    memset(&trace, 0, sizeof(trace));
    trace.fixture = &fixture;
    trace.stop_at = 0;
    trace.stop_value = 7;
    expected[0] = &fixture.nodes[1];
    expected[1] = &fixture.nodes[2];
    expected[2] = &fixture.nodes[0];
    VisitCircularListFrom(&fixture.owner, &fixture.nodes[1], record_visit, &trace);
    check_trace(&trace, expected, 3);
    assert(memcmp(&fixture, &before, sizeof(fixture)) == 0);

    /* A callback redirect of the current node's next link changes the trace. */
    make_fixture(&fixture, 6);
    expected_fixture = fixture;
    expected_fixture.nodes[1].next = &fixture.nodes[4];
    memset(&trace, 0, sizeof(trace));
    trace.fixture = &fixture;
    trace.stop_at = 99;
    trace.redirect_next = 1;
    trace.next_replacement = &fixture.nodes[4];
    expected[0] = &fixture.nodes[1];
    expected[1] = &fixture.nodes[4];
    expected[2] = &fixture.nodes[5];
    expected[3] = &fixture.nodes[0];
    VisitCircularListFrom(&fixture.owner, &fixture.nodes[1], record_visit, &trace);
    check_trace(&trace, expected, 4);
    assert(memcmp(&fixture, &expected_fixture, sizeof(fixture)) == 0);

    /* The owner head is loaded at wrap time, after callback mutation. */
    make_fixture(&fixture, 3);
    expected_fixture = fixture;
    expected_fixture.owner.list_head = &fixture.nodes[4];
    expected_fixture.nodes[4].next = &fixture.nodes[5];
    expected_fixture.nodes[5].next = &fixture.nodes[2];
    memset(&trace, 0, sizeof(trace));
    trace.fixture = &fixture;
    trace.stop_at = 99;
    trace.replace_head = 1;
    trace.mutate_at = 0;
    trace.head_replacement = &fixture.nodes[4];
    fixture.nodes[4].next = &fixture.nodes[5];
    fixture.nodes[5].next = &fixture.nodes[2];
    before = fixture;
    expected_fixture.owner.list_head = &fixture.nodes[4];
    expected[0] = &fixture.nodes[2];
    expected[1] = &fixture.nodes[4];
    expected[2] = &fixture.nodes[5];
    VisitCircularListFrom(&fixture.owner, &fixture.nodes[2], record_visit, &trace);
    check_trace(&trace, expected, 3);
    assert(memcmp(&fixture, &expected_fixture, sizeof(fixture)) == 0);
    return 0;
}
