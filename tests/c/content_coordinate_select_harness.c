#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "content_coordinate_select.h"
#include "container.h"
#include "list_accessors.h"
#include "list_count.h"
#include "list_visit.h"

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
typedef char EntryPosition08TargetOffset[
    sizeof(void *) != 4 ||
    offsetof(struct ContentConsumerEntry, position08) == 8 ? 1 : -1];
typedef char EntryPosition0CTargetOffset[
    sizeof(void *) != 4 ||
    offsetof(struct ContentConsumerEntry, position0C) == 12 ? 1 : -1];
typedef char EntryValue5cTargetOffset[
    sizeof(void *) != 4 ||
    offsetof(struct ContentConsumerEntry, value5c) == 0x5c ? 1 : -1];
typedef char EntryPosition08NativeOffset[
    offsetof(struct ContentConsumerEntry, position08) ==
        8 + sizeof(void *) - 4 ? 1 : -1];
typedef char EntryPosition0CNativeOffset[
    offsetof(struct ContentConsumerEntry, position0C) ==
        12 + sizeof(void *) - 4 ? 1 : -1];
typedef char EntryValue5cNativeOffset[
    offsetof(struct ContentConsumerEntry, value5c) ==
        0x5c + sizeof(void *) - 4 ? 1 : -1];
typedef char QuerySelectorTargetOffset[
    sizeof(void *) != 4 || offsetof(struct ContentCoordinateQuery, x) == 4 ?
    1 : -1];
typedef char QuerySelectedTargetOffset[
    sizeof(void *) != 4 ||
    offsetof(struct ContentCoordinateQuery, selected) == 8 ? 1 : -1];
typedef char QuerySelectorsNativeOffsets[
    offsetof(struct ContentCoordinateQuery, x) == 4 &&
    offsetof(struct ContentCoordinateQuery, y) == 5 ? 1 : -1];
typedef char QuerySelectedNativeOffset[
    offsetof(struct ContentCoordinateQuery, selected) == 8 ? 1 : -1];

static void initialize_fixture(struct Fixture *fixture, unsigned count)
{
    unsigned i;

    memset(fixture, 0xA5, sizeof(*fixture));
    memset(&fixture->owner, 0, sizeof(fixture->owner));
    fixture->owner.list_head = count == 0 ? 0 : &fixture->nodes[0];
    for (i = 0; i < NODE_COUNT; i++) {
        fixture->entries[i].object = &fixture->units[i];
        fixture->nodes[i].unknown00 = (ListPayload)(unsigned long)
            &fixture->entries[i];
        fixture->nodes[i].previous = i == 0 ? 0 : &fixture->nodes[i - 1];
        fixture->nodes[i].next = i + 1 == count ? 0 : &fixture->nodes[i + 1];
    }
    fixture->query.unknown00 = 0x13579BDFu;
    fixture->query.x = 17;
    fixture->query.y = -19;
    fixture->query.selected = &fixture->entries[2];
}

struct BoundaryCase {
    signed short position;
    signed char shifted;
    int representable_match;
};

static const struct BoundaryCase sBoundaries[] = {
    {-32768, -128, 0},
    {-4097, -128, 0},
    {-4096, -128, 1},
    {-33, -2, 1},
    {-32, -1, 1},
    {-1, -1, 1},
    {0, 0, 1},
    {31, 0, 1},
    {32, 1, 1},
    {4063, 126, 1},
    {4095, 127, 1},
    {4096, 0, 0},
    {32767, 0, 0},
};

static void check_boundary_axis(unsigned axis)
{
    unsigned i;

    for (i = 0; i < sizeof(sBoundaries) / sizeof(sBoundaries[0]); i++) {
        struct Fixture fixture;
        struct Fixture expected;
        int result;

        initialize_fixture(&fixture, 1);
        fixture.entries[0].position08 = axis == 0 ?
            sBoundaries[i].position : 0;
        fixture.entries[0].position0C = axis == 1 ?
            sBoundaries[i].position : 0;
        fixture.query.x = axis == 0 ? sBoundaries[i].shifted : 0;
        fixture.query.y = axis == 1 ? sBoundaries[i].shifted : 0;
        expected = fixture;
        if (sBoundaries[i].representable_match)
            expected.query.selected = &fixture.entries[0];

        result = ContentCoordinateSelectCallback(&fixture.nodes[0],
                                                 &fixture.query);
        assert(result == (sBoundaries[i].representable_match ? 0u : 1u));
        assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
    }
}

static void check_independent_axis_mismatches(void)
{
    struct Fixture fixture;
    struct Fixture expected;

    initialize_fixture(&fixture, 1);
    fixture.entries[0].position08 = 64;
    fixture.entries[0].position0C = -64;
    fixture.query.x = 3;
    fixture.query.y = -2;
    expected = fixture;
    assert(ContentCoordinateSelectCallback(&fixture.nodes[0],
                                           &fixture.query) == 1);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);

    fixture.query.x = 2;
    fixture.query.y = -1;
    expected = fixture;
    assert(ContentCoordinateSelectCallback(&fixture.nodes[0],
                                           &fixture.query) == 1);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
}

static void check_visitor_first_match_and_empty_list(void)
{
    struct Fixture fixture;
    struct Fixture expected;

    initialize_fixture(&fixture, NODE_COUNT);
    fixture.entries[0].position08 = 96;
    fixture.entries[0].position0C = 96;
    fixture.entries[1].position08 = 64;
    fixture.entries[1].position0C = 96;
    fixture.entries[2].position08 = 64;
    fixture.entries[2].position0C = 96;
    fixture.query.x = 2;
    fixture.query.y = 3;
    expected = fixture;
    expected.query.selected = &fixture.entries[1];

    VisitLinkedNodes(&fixture.owner, ContentCoordinateSelectCallback,
                     &fixture.query);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);

    initialize_fixture(&fixture, 0);
    expected = fixture;
    VisitLinkedNodes(&fixture.owner, ContentCoordinateSelectCallback,
                     &fixture.query);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
}

int main(void)
{
    assert(sizeof(void *) == 4 || sizeof(void *) == 8);
    check_boundary_axis(0);
    check_boundary_axis(1);
    check_independent_axis_mismatches();
    check_visitor_first_match_and_empty_list();
    return 0;
}
