#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "content_unit_coordinate_select.h"
#include "list_accessors.h"

#define NODE_COUNT 3

struct Fixture {
    struct Container owner;
    struct ListCountNode nodes[NODE_COUNT];
    struct ContentConsumerEntry entries[NODE_COUNT];
    struct UnitFieldView units[NODE_COUNT];
    struct ContentCoordinateQuery query;
};

typedef char EntryObjectAtZero[
    offsetof(struct ContentConsumerEntry, object) == 0 ? 1 : -1];
typedef char UnitXAtF6[
    offsetof(struct UnitFieldView, bytesF1) + 5 == 0xF6 ? 1 : -1];
typedef char UnitYAtF7[
    offsetof(struct UnitFieldView, bytesF1) + 6 == 0xF7 ? 1 : -1];
typedef char QueryXAtTarget4[
    offsetof(struct ContentCoordinateQuery, x) == 4 ? 1 : -1];
typedef char QueryYImmediatelyAfterX[
    offsetof(struct ContentCoordinateQuery, y) ==
        offsetof(struct ContentCoordinateQuery, x) + 1 ? 1 : -1];
typedef char QuerySelectedAtNativeTargetOffset[
    offsetof(struct ContentCoordinateQuery, selected) ==
        8 ? 1 : -1];
typedef char NodePayloadAtZero[
    offsetof(struct ListCountNode, unknown00) == 0 ? 1 : -1];

static const signed char query_values[5] = {-128, -1, 0, 1, 127};
static const u8 unit_values[5] = {0, 1, 127, 128, 255};
/* Rows are signed queries -128, -1, 0, 1, 127; columns are unit bytes
 * 0, 1, 127, 128, 255. These fixed values distinguish signed from unsigned. */
static const u8 expected_equal[5][5] = {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {1, 0, 0, 0, 0},
    {0, 1, 0, 0, 0},
    {0, 0, 1, 0, 0},
};

static void initialize_fixture(struct Fixture *fixture)
{
    unsigned i;

    memset(fixture, 0xA5, sizeof(*fixture));
    fixture->owner.allocator_owner = (void *)(unsigned long)0x13579BDFUL;
    fixture->owner.list_head = &fixture->nodes[0];
    fixture->owner.tree_root = (struct TreeCountNode *)(unsigned long)0x2468ACE0UL;
    for (i = 0; i < NODE_COUNT; i++) {
        fixture->nodes[i].unknown00 = (ListPayload)(unsigned long)&fixture->entries[i];
        fixture->nodes[i].previous = i == 0 ? 0 : &fixture->nodes[i - 1];
        fixture->nodes[i].next = i + 1 == NODE_COUNT ? 0 : &fixture->nodes[i + 1];
        fixture->entries[i].object = &fixture->units[i];
    }
    fixture->query.unknown00 = 0x10293847UL;
    fixture->query.x = 1;
    fixture->query.y = 1;
    fixture->query.unknown06[0] = 0x5A;
    fixture->query.unknown06[1] = 0xC3;
    fixture->query.selected = &fixture->entries[2];
}

static void set_unit_xy(struct Fixture *fixture, unsigned index, u8 x, u8 y)
{
    fixture->units[index].bytesF1[5] = x;
    fixture->units[index].bytesF1[6] = y;
}

static void check_signed_query_and_unsigned_unit_bytes(void)
{
    unsigned axis;
    unsigned q;
    unsigned v;

    for (axis = 0; axis < 2; axis++) {
        for (q = 0; q < 5; q++) {
            for (v = 0; v < 5; v++) {
                struct Fixture actual;
                struct Fixture expected;
                u32 result;

                initialize_fixture(&actual);
                actual.query.x = 1;
                actual.query.y = 1;
                set_unit_xy(&actual, 0, 1, 1);
                if (axis == 0) {
                    actual.query.x = query_values[q];
                    actual.units[0].bytesF1[5] = unit_values[v];
                } else {
                    actual.query.y = query_values[q];
                    actual.units[0].bytesF1[6] = unit_values[v];
                }
                expected = actual;
                if (expected_equal[q][v])
                    expected.query.selected = &actual.entries[0];

                result = ContentUnitCoordinateSelectCallback(
                    &actual.nodes[0], &actual.query);
                assert(result == (expected_equal[q][v] ? 0u : 1u));
                assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
            }
        }
    }
}

static void check_direct_callback_return_and_output_sentinel(void)
{
    struct Fixture actual;
    struct Fixture expected;
    u32 result;

    initialize_fixture(&actual);
    set_unit_xy(&actual, 0, 128, 255);
    actual.query.x = -128;
    actual.query.y = -1;
    expected = actual;

    result = ContentUnitCoordinateSelectCallback(&actual.nodes[0],
                                                  &actual.query);
    assert(result == 1);
    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);

    actual.query.x = 127;
    actual.query.y = 127;
    set_unit_xy(&actual, 0, 127, 127);
    expected = actual;
    expected.query.selected = &actual.entries[0];
    result = ContentUnitCoordinateSelectCallback(&actual.nodes[0],
                                                  &actual.query);
    assert(result == 0);
    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
}

static void check_real_visitor_first_match_absence_and_duplicates(void)
{
    struct Fixture actual;
    struct Fixture expected;

    initialize_fixture(&actual);
    actual.query.x = 1;
    actual.query.y = 1;
    set_unit_xy(&actual, 0, 0, 1);
    set_unit_xy(&actual, 1, 1, 1);
    set_unit_xy(&actual, 2, 1, 1);
    expected = actual;
    expected.query.selected = &actual.entries[1];
    VisitLinkedNodes(&actual.owner, ContentUnitCoordinateSelectCallback,
                     &actual.query);
    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);

    initialize_fixture(&actual);
    actual.query.x = 127;
    actual.query.y = 127;
    set_unit_xy(&actual, 0, 0, 127);
    set_unit_xy(&actual, 1, 1, 127);
    set_unit_xy(&actual, 2, 255, 127);
    expected = actual;
    VisitLinkedNodes(&actual.owner, ContentUnitCoordinateSelectCallback,
                     &actual.query);
    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);

    initialize_fixture(&actual);
    actual.query.x = 0;
    actual.query.y = 0;
    set_unit_xy(&actual, 0, 0, 0);
    set_unit_xy(&actual, 1, 0, 0);
    set_unit_xy(&actual, 2, 0, 0);
    expected = actual;
    expected.query.selected = &actual.entries[0];
    VisitLinkedNodes(&actual.owner, ContentUnitCoordinateSelectCallback,
                     &actual.query);
    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
}

int main(void)
{
    assert(sizeof(void *) == 4 || sizeof(void *) == 8);
    check_signed_query_and_unsigned_unit_bytes();
    check_direct_callback_return_and_output_sentinel();
    check_real_visitor_first_match_absence_and_duplicates();
    return 0;
}
