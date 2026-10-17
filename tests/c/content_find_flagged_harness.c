#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "content_find_flagged.h"
#include "content_unit_select.h"
#include "list_count.h"
#include "unit.h"

#define SLOT_COUNT 3
#define ENTRY_COUNT 4

struct Fixture {
    struct ListCountContext context;
    struct Container owners[SLOT_COUNT];
    struct ListCountNode nodes[SLOT_COUNT][ENTRY_COUNT];
    struct ContentConsumerEntry entries[SLOT_COUNT][ENTRY_COUNT];
    struct UnitFieldView units[SLOT_COUNT][ENTRY_COUNT];
};

typedef char Context10TargetOffset[
    sizeof(void *) != 4 || offsetof(struct ListCountContext, unknown10) == 0x10
        ? 1 : -1];
typedef char EntryUnitPointerAtTargetOffset[
    sizeof(void *) != 4 || offsetof(struct ContentConsumerEntry, object) == 0
        ? 1 : -1];

static void initialize_fixture(struct Fixture *fixture)
{
    unsigned slot;
    unsigned index;

    memset(fixture, 0xA5, sizeof(*fixture));
    memset(&fixture->context, 0x31, sizeof(fixture->context));
    for (slot = 0; slot < SLOT_COUNT; slot++) {
        fixture->owners[slot].allocator_owner = &fixture->entries[slot][0];
        fixture->owners[slot].list_head = &fixture->nodes[slot][0];
        fixture->owners[slot].tree_root = 0;
        for (index = 0; index < ENTRY_COUNT; index++) {
            fixture->entries[slot][index].object = &fixture->units[slot][index];
            fixture->nodes[slot][index].unknown00 =
                (ListPayload)(unsigned long)&fixture->entries[slot][index];
            fixture->nodes[slot][index].previous = index == 0 ? 0 :
                &fixture->nodes[slot][index - 1];
            fixture->nodes[slot][index].next = index + 1 == ENTRY_COUNT ? 0 :
                &fixture->nodes[slot][index + 1];
            fixture->units[slot][index].half18[8] = 0;
        }
    }
    fixture->context.unknown10 = &fixture->owners[0];
    fixture->context.unknown14 = &fixture->owners[1];
    fixture->context.unknown18 = &fixture->owners[2];
}

static void assert_unchanged(struct Fixture *actual,
                             const struct Fixture *before)
{
    assert(memcmp(actual, before, sizeof(*actual)) == 0);
}

static void assert_result(struct Fixture *fixture,
                          struct ContentConsumerEntry *expected)
{
    struct Fixture before;
    struct ContentConsumerEntry *actual;

    memcpy(&before, fixture, sizeof(before));
    actual = FindContentEntryWithUnitFlag12(&fixture->context);
    assert(actual == expected);
    assert_unchanged(fixture, &before);
}

static void test_empty_and_no_match(void)
{
    struct Fixture fixture;
    struct Fixture before;
    struct ContentConsumerEntry *result;

    initialize_fixture(&fixture);
    fixture.owners[0].list_head = 0;
    memcpy(&before, &fixture, sizeof(before));
    result = FindContentEntryWithUnitFlag12(&fixture.context);
    assert(result == 0);
    assert_unchanged(&fixture, &before);

    initialize_fixture(&fixture);
    fixture.units[0][0].half18[8] = 0xA800; /* Adjacent bits, not bit 12. */
    fixture.units[0][1].half18[8] = 0x0800;
    fixture.units[0][2].half18[8] = 0x2000;
    fixture.entries[0][3].object = 0;
    memcpy(&before, &fixture, sizeof(before));
    result = FindContentEntryWithUnitFlag12(&fixture.context);
    assert(result == 0);
    assert_unchanged(&fixture, &before);
}

static void test_each_match_position_and_duplicate_first_match(void)
{
    static const unsigned positions[] = {0, 1, 3};
    unsigned scenario;

    for (scenario = 0;
         scenario < sizeof(positions) / sizeof(positions[0]);
         scenario++) {
        struct Fixture fixture;
        unsigned match = positions[scenario];

        initialize_fixture(&fixture);
        fixture.units[0][match].half18[8] = 0x1000;
        assert_result(&fixture, &fixture.entries[0][match]);
    }

    {
        struct Fixture fixture;
        initialize_fixture(&fixture);
        fixture.units[0][0].half18[8] = 0x1000;
        fixture.units[0][1].half18[8] = 0x5000;
        fixture.units[0][2].half18[8] = 0x1000;
        assert_result(&fixture, &fixture.entries[0][0]);
    }
}

static void test_only_context_field10_is_visited(void)
{
    struct Fixture fixture;
    initialize_fixture(&fixture);
    fixture.units[1][1].half18[8] = 0x1000;
    fixture.units[2][2].half18[8] = 0x1000;
    assert_result(&fixture, 0);

    fixture.units[0][2].half18[8] = 0x1000;
    assert_result(&fixture, &fixture.entries[0][2]);
}

int main(void)
{
    test_empty_and_no_match();
    test_each_match_position_and_duplicate_first_match();
    test_only_context_field10_is_visited();
    return 0;
}
