#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "content_selection_lookup.h"
#include "container.h"
#include "list_accessors.h"
#include "list_count.h"
#include "list_visit.h"

typedef char EntryObjectAtZero[
    offsetof(struct ContentConsumerEntry, object) == 0 ? 1 : -1];
typedef char EntryValue5cAtTargetOffset[
    sizeof(void *) != 4 ||
    offsetof(struct ContentConsumerEntry, value5c) == 0x5c ? 1 : -1];
typedef char EntryNativeValue5cOffset[
    offsetof(struct ContentConsumerEntry, value5c) ==
        0x5c + sizeof(void *) - 4 ? 1 : -1];
typedef char LookupContextTargetResultOffset[
    sizeof(void *) != 4 ||
    offsetof(struct ContentSelectionLookupContext, selected) == 8 ? 1 : -1];
typedef char LookupContextNativeResultOffset[
    offsetof(struct ContentSelectionLookupContext, selected) ==
        ((2 * sizeof(void *) + 3) & ~(sizeof(void *) - 1)) ? 1 : -1];

struct Fixture {
    struct ListCountContext context;
    struct Container owner;
    struct ListCountNode nodes[4];
    struct ContentConsumerEntry entries[4];
    struct UnitFieldView units[5];
};

static void initialize_fixture(struct Fixture *fixture, int duplicate)
{
    unsigned i;

    memset(fixture, 0xA5, sizeof(*fixture));
    memset(&fixture->owner, 0, sizeof(fixture->owner));
    memset(fixture->nodes, 0, sizeof(fixture->nodes));
    memset(fixture->entries, 0x6D, sizeof(fixture->entries));
    memset(fixture->units, 0x39, sizeof(fixture->units));

    fixture->context.unknown10 = &fixture->owner;
    fixture->owner.list_head = &fixture->nodes[0];
    for (i = 0; i < 4; i++) {
        fixture->entries[i].object = &fixture->units[i];
        fixture->nodes[i].unknown00 = (ListPayload)(unsigned long)
            &fixture->entries[i];
        fixture->nodes[i].previous = i == 0 ? 0 : &fixture->nodes[i - 1];
        fixture->nodes[i].next = i == 3 ? 0 : &fixture->nodes[i + 1];
    }
    if (duplicate)
        fixture->entries[3].object = &fixture->units[1];
}

static void check_predicate_match(void)
{
    struct Fixture fixture;
    struct Fixture expected;
    struct ContentSelectionLookupContext context;
    struct ContentSelectionLookupContext expected_context;
    u32 result;

    initialize_fixture(&fixture, 0);
    memset(&context, 0xB7, sizeof(context));
    context.selector = &fixture.units[2];
    context.unknown04 = 0x13579BDFu;
    context.selected = &fixture.entries[0];
    expected = fixture;
    expected_context = context;
    expected_context.selected = &fixture.entries[2];

    result = ContentSelectionEqualsObject(&fixture.nodes[2], &context);
    assert(result == 0);
    assert(memcmp(&context, &expected_context, sizeof(context)) == 0);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
}

static void check_predicate_nonmatch_preserves_output(void)
{
    struct Fixture fixture;
    struct Fixture expected;
    struct ContentSelectionLookupContext context;
    struct ContentSelectionLookupContext expected_context;
    u32 result;

    initialize_fixture(&fixture, 0);
    memset(&context, 0xC3, sizeof(context));
    context.selector = &fixture.units[4];
    context.unknown04 = 0x2468ACE0u;
    context.selected = &fixture.entries[3];
    expected = fixture;
    expected_context = context;

    result = ContentSelectionEqualsObject(&fixture.nodes[1], &context);
    assert(result == 1);
    assert(memcmp(&context, &expected_context, sizeof(context)) == 0);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
}

static void check_null_selector_matches_null_object(void)
{
    struct Fixture fixture;
    struct Fixture expected;
    struct ContentSelectionLookupContext context;
    struct ContentSelectionLookupContext expected_context;
    u32 result;

    initialize_fixture(&fixture, 0);
    fixture.entries[2].object = 0;
    memset(&context, 0x91, sizeof(context));
    context.selector = 0;
    context.unknown04 = 0xA1B2C3D4u;
    context.selected = &fixture.entries[0];
    expected = fixture;
    expected_context = context;
    expected_context.selected = &fixture.entries[2];

    result = ContentSelectionEqualsObject(&fixture.nodes[2], &context);
    assert(result == 0);
    assert(memcmp(&context, &expected_context, sizeof(context)) == 0);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
}

static void check_wrapper_position(unsigned selected_index, int duplicate)
{
    struct Fixture fixture;
    struct Fixture expected;
    struct ContentConsumerEntry *selected;
    const unsigned expected_index = duplicate ? 1 : selected_index;

    initialize_fixture(&fixture, duplicate);
    expected = fixture;
    selected = ResolveContentConsumerSelection(
        &fixture.context, fixture.entries[selected_index].object);
    assert(selected == &fixture.entries[expected_index]);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
}

static void check_wrapper_no_match_returns_null(void)
{
    struct Fixture fixture;
    struct Fixture expected;
    struct ContentConsumerEntry *selected;

    initialize_fixture(&fixture, 0);
    expected = fixture;
    selected = ResolveContentConsumerSelection(&fixture.context,
                                                &fixture.units[4]);
    assert(selected == 0);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
}

static void check_empty_owner_returns_null(void)
{
    struct Fixture fixture;
    struct Fixture expected;
    struct ContentConsumerEntry *selected;

    initialize_fixture(&fixture, 0);
    fixture.owner.list_head = 0;
    expected = fixture;
    selected = ResolveContentConsumerSelection(&fixture.context,
                                                &fixture.units[0]);
    assert(selected == 0);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
}

static void check_wrapper_null_selector_matches_null_object(void)
{
    struct Fixture fixture;
    struct Fixture expected;
    struct ContentConsumerEntry *selected;

    initialize_fixture(&fixture, 0);
    fixture.entries[2].object = 0;
    expected = fixture;
    selected = ResolveContentConsumerSelection(&fixture.context, 0);
    assert(selected == &fixture.entries[2]);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
}

int main(void)
{
    assert(sizeof(void *) == 4 || sizeof(void *) == 8);
    check_predicate_match();
    check_predicate_nonmatch_preserves_output();
    check_null_selector_matches_null_object();
    check_wrapper_no_match_returns_null();
    check_empty_owner_returns_null();
    check_wrapper_null_selector_matches_null_object();
    check_wrapper_position(0, 0);
    check_wrapper_position(1, 0);
    check_wrapper_position(2, 0);
    check_wrapper_position(3, 0);
    check_wrapper_position(3, 1);
    return 0;
}
