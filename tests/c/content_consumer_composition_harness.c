#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "container.h"
#include "content_consumer.h"
#include "content_globals.h"
#include "content_record.h"
#include "content_state.h"
#include "list_count.h"
#include "list_count_wrappers.h"

#define NODE_COUNT 13

struct Fixture {
    struct ContentConsumerContext input;
    struct ListCountContext context;
    struct Container owners[3];
    struct ListCountNode nodes[3][NODE_COUNT];
    struct ContentConsumerEntry entries[NODE_COUNT];
    struct UnitFieldView units[NODE_COUNT];
    struct ContentGlobalPointerOwner global_owner;
    u16 first_output;
    u16 second_output;
};

struct ListCountContext *gContentConsumerContext;
struct ContentGlobalPointerOwner *gContentGlobalPointerOwner;
const struct ContentRecord gContentRecords[2] = {
    { {0x11}, 0, {0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40} },
    { {0x22}, 0, {0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae} }
};
struct ContentIndexState gContentState __attribute__((aligned(4)));

static void initialize_fixture(struct Fixture *fixture)
{
    unsigned list, index;

    memset(fixture, 0, sizeof(*fixture));
    memset(&gContentState, 0x65, sizeof(gContentState));
    gContentState.currentIndex = 0;
    fixture->first_output = 0xbeef;
    fixture->second_output = 0xdead;
    fixture->context.unknown10 = &fixture->owners[0];
    fixture->context.unknown14 = &fixture->owners[1];
    fixture->context.unknown18 = &fixture->owners[2];
    fixture->global_owner.unknown00 = 0x13579bdf;
    fixture->global_owner.unknown04 = &fixture->context;
    fixture->global_owner.flagsD0 = 0x2468ace0;
    gContentConsumerContext = &fixture->context;
    gContentGlobalPointerOwner = &fixture->global_owner;

    for (list = 0; list < 3; list++) {
        for (index = 0; index < NODE_COUNT; index++) {
            fixture->nodes[list][index].unknown00 = 0;
            fixture->nodes[list][index].previous =
                index == 0 ? 0 : &fixture->nodes[list][index - 1];
            fixture->nodes[list][index].next =
                index + 1 == NODE_COUNT ? 0 :
                &fixture->nodes[list][index + 1];
        }
    }
    for (index = 0; index < NODE_COUNT; index++) {
        fixture->entries[index].object = &fixture->units[index];
        fixture->entries[index].unknown04 = 0x10203040u + index;
        fixture->entries[index].position08 = (signed short)(-20 + index);
        fixture->entries[index].position0C = (signed short)(30 + index);
        fixture->entries[index].value5c = (u16)(0x5100 + index);
        fixture->units[index].half18[8] = (u16)(0x100 + index);
        fixture->units[index].half18[13] = (u16)(0x200 + index);
        fixture->nodes[0][index].unknown00 =
            (ListPayload)(unsigned long)&fixture->entries[index];
        fixture->nodes[1][index].unknown00 =
            (ListPayload)(unsigned long)&fixture->entries[index];
        fixture->nodes[2][index].unknown00 =
            (ListPayload)(unsigned long)&fixture->entries[index];
    }
}

static void link_length(struct Fixture *fixture, unsigned list,
                        unsigned length)
{
    assert(list < 3);
    assert(length <= NODE_COUNT);
    fixture->owners[list].list_head =
        length == 0 ? 0 : &fixture->nodes[list][0];
    if (length != 0)
        fixture->nodes[list][length - 1].next = 0;
}

static void assert_call(struct Fixture *fixture,
                        const void *selector,
                        u16 expected_first, u16 expected_second)
{
    struct Fixture expected;
    struct ContentIndexState expected_state = gContentState;
    struct ContentRecord expected_records[2];
    struct ListCountContext *expected_context_global = gContentConsumerContext;
    struct ContentGlobalPointerOwner *expected_owner_global =
        gContentGlobalPointerOwner;

    memcpy(&expected, fixture, sizeof(expected));
    memcpy(expected_records, gContentRecords, sizeof(expected_records));
    expected.first_output = expected_first;
    expected.second_output = expected_second;
    ConsumeContentRecord(&fixture->input, selector,
                         &fixture->first_output, &fixture->second_output);
    assert(memcmp(fixture, &expected, sizeof(expected)) == 0);
    assert(gContentConsumerContext == expected_context_global);
    assert(gContentGlobalPointerOwner == expected_owner_global);
    assert(memcmp(&gContentState, &expected_state, sizeof(expected_state)) == 0);
    assert(memcmp(gContentRecords, expected_records, sizeof(expected_records)) == 0);
}

static void check_flagged_list_lengths_and_record_indices(void)
{
    static const unsigned lengths[] = {0, 1, 13};
    static const u16 expected_qualifying[] = {0, 0, 3};
    unsigned test;

    for (test = 0; test < 3; test++) {
        struct Fixture fixture;
        unsigned i;

        initialize_fixture(&fixture);
        fixture.input.flags = 0x10;
        link_length(&fixture, 1, lengths[test]);
        gContentState.currentIndex = test == 2 ? 1 : 0;
        if (test == 1) {
            fixture.entries[0].object = 0;
        } else if (test == 2) {
            for (i = 0; i < NODE_COUNT; i++) {
                /* Count bit 0 separately from field 27's bit 12. */
                fixture.units[i].half18[8] =
                    (i == 0 || i == 4 || i == 12) ? 0x8001 : 0x8000;
                fixture.units[i].half18[8] |=
                    (i == 1 || i == 4 || i == 11) ? 0x1000 : 0;
            }
            fixture.entries[3].object = 0;
        }
        assert_call(&fixture, &fixture.input, expected_qualifying[test],
                    test == 2 ? 0xa7 : 0x39);
    }
}

static void check_selection_wrap_duplicate_and_real_counts(void)
{
    struct Fixture fixture;

    initialize_fixture(&fixture);
    fixture.input.flags = 0;
    link_length(&fixture, 0, 3);
    link_length(&fixture, 1, 3);
    link_length(&fixture, 2, 2);
    fixture.entries[0].object = &fixture.units[7];
    fixture.entries[0].value5c = 0xffff;
    fixture.entries[1].object = &fixture.units[7];
    fixture.entries[1].value5c = 0x4321;
    fixture.entries[2].object = 0;
    fixture.nodes[0][0].unknown00 =
        (ListPayload)(unsigned long)&fixture.entries[2];
    fixture.nodes[0][1].unknown00 =
        (ListPayload)(unsigned long)&fixture.entries[0];
    fixture.nodes[0][2].unknown00 =
        (ListPayload)(unsigned long)&fixture.entries[1];
    fixture.units[7].half18[8] = 0x1000;
    assert_call(&fixture, &fixture.units[7], 0, 6);
    /* The first duplicate wraps; list counts are 3 + 2 + found. */
}

static void check_selection_nonwrap_and_no_flag12_match(void)
{
    struct Fixture fixture;

    initialize_fixture(&fixture);
    fixture.input.flags = 0;
    link_length(&fixture, 0, 2);
    link_length(&fixture, 1, 3);
    link_length(&fixture, 2, 2);
    fixture.entries[0].object = &fixture.units[8];
    fixture.entries[0].value5c = 0x1234;
    fixture.entries[1].object = &fixture.units[9];
    fixture.units[8].half18[8] = 0;
    fixture.units[9].half18[8] = 0;
    assert_call(&fixture, &fixture.units[8], 0x1235, 5);
    /* Counts are 3 + 2 + no matching unit. */
}

int main(void)
{
    assert(offsetof(struct ContentConsumerContext, flags) == 16);
    assert(offsetof(struct UnitFieldView, half18) + 8 * sizeof(u16) == 40);
    check_flagged_list_lengths_and_record_indices();
    check_selection_wrap_duplicate_and_real_counts();
    check_selection_nonwrap_and_no_flag12_match();
    return 0;
}
