#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "content_other_selection.h"
#include "list_accessors.h"

typedef char LookupResultTargetOffset[
    sizeof(void *) != 4 ||
    offsetof(struct ContentSelectionLookupContext, selected) == 8 ? 1 : -1];
typedef char LookupResultNativeOffset[
    offsetof(struct ContentSelectionLookupContext, selected) ==
        ((2 * sizeof(void *) + 3) & ~(sizeof(void *) - 1)) ? 1 : -1];
typedef char EntryObjectAtZero[
    offsetof(struct ContentConsumerEntry, object) == 0 ? 1 : -1];

enum { LIST_COUNT = 3, NODE_COUNT = 4, ENTRY_COUNT = 12, UNIT_COUNT = 13 };

struct Fixture {
    struct ListCountContext context;
    struct Container owners[LIST_COUNT];
    struct ListCountNode nodes[LIST_COUNT][NODE_COUNT];
    struct ContentConsumerEntry entries[ENTRY_COUNT];
    struct UnitFieldView units[UNIT_COUNT];
};

static void initialize_fixture(struct Fixture *fixture)
{
    unsigned list_index;
    unsigned node_index;

    memset(fixture, 0xA7, sizeof(*fixture));
    memset(fixture->owners, 0, sizeof(fixture->owners));
    memset(fixture->nodes, 0, sizeof(fixture->nodes));
    memset(fixture->entries, 0x5C, sizeof(fixture->entries));
    memset(fixture->units, 0x39, sizeof(fixture->units));

    fixture->context.unknown10 = &fixture->owners[0];
    fixture->context.unknown14 = &fixture->owners[1];
    fixture->context.unknown18 = &fixture->owners[2];
    for (list_index = 0; list_index < LIST_COUNT; list_index++) {
        fixture->owners[list_index].list_head =
            &fixture->nodes[list_index][0];
        for (node_index = 0; node_index < NODE_COUNT; node_index++) {
            unsigned entry_index = list_index * NODE_COUNT + node_index;
            fixture->entries[entry_index].object =
                &fixture->units[entry_index];
            fixture->nodes[list_index][node_index].unknown00 =
                (ListPayload)(unsigned long)&fixture->entries[entry_index];
            fixture->nodes[list_index][node_index].previous =
                node_index == 0 ? 0 :
                    &fixture->nodes[list_index][node_index - 1];
            fixture->nodes[list_index][node_index].next =
                node_index + 1 == NODE_COUNT ? 0 :
                    &fixture->nodes[list_index][node_index + 1];
        }
    }
}

static struct ContentConsumerEntry *resolve_at14(
    struct ListCountContext *context, const void *selector)
{
    return ResolveContentSelectionAt14(context, selector);
}

static struct ContentConsumerEntry *resolve_at18(
    struct ListCountContext *context, const void *selector)
{
    return ResolveContentSelectionAt18(context, selector);
}

typedef struct ContentConsumerEntry *(*ResolveSelection)(
    struct ListCountContext *context, const void *selector);

static void check_case(ResolveSelection resolve, unsigned selected_list,
                       int target_index, int duplicate, int empty,
                       int null_object)
{
    struct Fixture fixture;
    struct Fixture expected;
    const void *selector;
    struct ContentConsumerEntry *actual;
    struct ContentConsumerEntry *expected_result = 0;
    unsigned target_entry = selected_list * NODE_COUNT;
    unsigned decoy_list;

    initialize_fixture(&fixture);

    selector = &fixture.units[UNIT_COUNT - 1];

    /* Each nonselected context list is a decoy, including +0x10. */
    for (decoy_list = 0; decoy_list < LIST_COUNT; decoy_list++) {
        if (decoy_list != selected_list)
            fixture.entries[decoy_list * NODE_COUNT].object =
                &fixture.units[UNIT_COUNT - 1];
    }
    if (target_index >= 0) {
        unsigned match = target_entry + (unsigned)target_index;
        fixture.entries[match].object = &fixture.units[UNIT_COUNT - 1];
        if (duplicate)
            fixture.entries[target_entry + NODE_COUNT - 1].object =
                &fixture.units[UNIT_COUNT - 1];
        expected_result = &fixture.entries[match];
    }
    if (null_object) {
        selector = 0;
        fixture.entries[target_entry + 1].object = 0;
        expected_result = target_index < 0 ? 0 :
                          &fixture.entries[target_entry + 1];
    }
    if (empty)
        fixture.owners[selected_list].list_head = 0;

    expected = fixture;
    actual = resolve(&fixture.context, selector);
    assert(actual == expected_result);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
}

static void check_wrapper(ResolveSelection resolve, unsigned selected_list)
{
    check_case(resolve, selected_list, -1, 0, 0, 0); /* absent */
    check_case(resolve, selected_list, -1, 0, 1, 0); /* empty */
    check_case(resolve, selected_list, 0, 0, 0, 0);  /* first */
    check_case(resolve, selected_list, 2, 0, 0, 0);  /* middle */
    check_case(resolve, selected_list, 1, 1, 0, 0);  /* first duplicate */
    check_case(resolve, selected_list, 1, 0, 0, 1);  /* NULL == NULL */
}

int main(void)
{
    check_wrapper(resolve_at14, 1);
    check_wrapper(resolve_at18, 2);
    return 0;
}
