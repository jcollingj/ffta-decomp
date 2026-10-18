#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "content_consumer.h"
#include "content_key_refresh.h"
#include "list_count.h"
#include "list_visit.h"
#include "tree_count.h"

#define OWNER_COUNT 3
#define ENTRY_COUNT 2

struct Fixture {
    struct ListCountContext context;
    struct Container owners[OWNER_COUNT];
    struct ListCountNode list_nodes[OWNER_COUNT][ENTRY_COUNT];
    struct TreeCountNode tree_nodes[OWNER_COUNT][ENTRY_COUNT];
    struct ContentConsumerEntry entries[OWNER_COUNT][ENTRY_COUNT];
};

static const u32 expected_keys[ENTRY_COUNT] = {
    0x80000001u, 0xFFFFFFFFu
};

static void initialize_fixture(struct Fixture *fixture)
{
    unsigned owner_index;
    unsigned entry_index;

    memset(fixture, 0xA5, sizeof(*fixture));
    memset(&fixture->context, 0x3C, sizeof(fixture->context));

    for (owner_index = 0; owner_index < OWNER_COUNT; owner_index++) {
        struct Container *owner = &fixture->owners[owner_index];

        owner->allocator_owner =
            (void *)(unsigned long)(0x1100 + owner_index * 0x100);
        owner->list_head = &fixture->list_nodes[owner_index][0];
        owner->tree_root = &fixture->tree_nodes[owner_index][1];
        for (entry_index = 0; entry_index < ENTRY_COUNT; entry_index++) {
            struct ContentConsumerEntry *entry =
                &fixture->entries[owner_index][entry_index];
            struct ListCountNode *list_node =
                &fixture->list_nodes[owner_index][entry_index];
            struct TreeCountNode *tree_node =
                &fixture->tree_nodes[owner_index][entry_index];

            entry->object = 0;
            entry->unknown24 = expected_keys[entry_index];

            list_node->unknown00 = (ListPayload)(unsigned long)entry;
            list_node->previous = entry_index == 0 ? 0 :
                &fixture->list_nodes[owner_index][entry_index - 1];
            list_node->next = entry_index + 1 == ENTRY_COUNT ? 0 :
                &fixture->list_nodes[owner_index][entry_index + 1];

            tree_node->unknown00 = 0x71000000u +
                owner_index * 0x100u + entry_index;
            tree_node->unknown04 = (ListPayload)(unsigned long)entry;
            tree_node->parent = entry_index == 0 ?
                &fixture->tree_nodes[owner_index][1] : 0;
            tree_node->child_a = entry_index == 1 ?
                &fixture->tree_nodes[owner_index][0] : 0;
            tree_node->child_b = 0;
            tree_node->flags = 0xA2000000u +
                owner_index * 0x100u + entry_index;
        }
    }
    fixture->context.unknown10 = &fixture->owners[0];
    fixture->context.unknown14 = &fixture->owners[1];
    fixture->context.unknown18 = &fixture->owners[2];
}

#ifdef CONTROLLED_CALLBACK

static struct Container **controlled_owner_slot;
static struct Container *replacement_owner;
struct ControlledTrace {
    unsigned events[3];
    void *payloads[2];
    void *owners[2];
    unsigned callback_count;
    unsigned rebuild_count;
    struct Container *rebuilt_owner;
};
static struct ControlledTrace trace;

int ContentRefreshTreeKeyCallback(void *payload, void *owner)
{
    unsigned index = trace.callback_count;

    assert(index < 2);
    trace.events[index] = index + 1;
    trace.payloads[index] = payload;
    trace.owners[index] = owner;
    trace.callback_count++;
    *controlled_owner_slot = replacement_owner;
    return 1;
}

void RebuildTreeByNumericKey(struct Container *owner)
{
    trace.events[trace.callback_count + trace.rebuild_count] = 3;
    trace.rebuilt_owner = owner;
    trace.rebuild_count++;
}

static void test_wrapper_reloads_owner_after_callback(void)
{
    typedef void (*RefreshWrapper)(const struct ListCountContext *);
    static const RefreshWrapper wrappers[OWNER_COUNT] = {
        RefreshContextTreeKeysAt10,
        RefreshContextTreeKeysAt14,
        RefreshContextTreeKeysAt18
    };
    unsigned owner_index;

    for (owner_index = 0; owner_index < OWNER_COUNT; owner_index++) {
        struct Fixture fixture;
        struct Fixture expected;
        struct Container replacement;
        struct Container expected_replacement;
        struct ControlledTrace expected_trace;

        initialize_fixture(&fixture);
        memset(&replacement, 0x6D, sizeof(replacement));
        expected_replacement = replacement;
        expected = fixture;
        memset(&trace, 0, sizeof(trace));
        memset(&expected_trace, 0, sizeof(expected_trace));
        controlled_owner_slot = owner_index == 0 ?
            &fixture.context.unknown10 : owner_index == 1 ?
            &fixture.context.unknown14 : &fixture.context.unknown18;
        replacement_owner = &replacement;
        expected_trace.events[0] = 1;
        expected_trace.events[1] = 2;
        expected_trace.events[2] = 3;
        expected_trace.payloads[0] = &fixture.entries[owner_index][0];
        expected_trace.payloads[1] = &fixture.entries[owner_index][1];
        expected_trace.owners[0] = &fixture.owners[owner_index];
        expected_trace.owners[1] = &fixture.owners[owner_index];
        expected_trace.callback_count = 2;
        expected_trace.rebuild_count = 1;
        expected_trace.rebuilt_owner = &replacement;

        wrappers[owner_index](&fixture.context);

        if (owner_index == 0)
            expected.context.unknown10 = &replacement;
        else if (owner_index == 1)
            expected.context.unknown14 = &replacement;
        else
            expected.context.unknown18 = &replacement;
        assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
        assert(memcmp(&trace, &expected_trace, sizeof(trace)) == 0);
        assert(memcmp(&replacement, &expected_replacement,
                      sizeof(replacement)) == 0);
    }
}

#else

static struct Container *rebuilt_owners[OWNER_COUNT];
static unsigned rebuild_count;

void RebuildTreeByNumericKey(struct Container *owner)
{
    assert(rebuild_count < OWNER_COUNT);
    rebuilt_owners[rebuild_count++] = owner;
}

static void test_callback_searches_and_sets_native_payload(void)
{
    struct Fixture fixture;
    struct Fixture expected;
    int result;

    initialize_fixture(&fixture);
    expected = fixture;
    expected.tree_nodes[0][0].unknown00 = expected_keys[0];
    result = ContentRefreshTreeKeyCallback(&fixture.entries[0][0],
                                           &fixture.owners[0]);

    assert(result == 1);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
}

static void test_callback_uses_first_in_order_payload_match(void)
{
    struct Fixture fixture;
    struct Fixture expected;
    int result;

    initialize_fixture(&fixture);
    fixture.tree_nodes[0][1].unknown04 =
        fixture.tree_nodes[0][0].unknown04;
    expected = fixture;
    expected.tree_nodes[0][0].unknown00 = expected_keys[0];
    result = ContentRefreshTreeKeyCallback(&fixture.entries[0][0],
                                           &fixture.owners[0]);

    assert(result == 1);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
}

static void test_three_wrappers_compose_real_callback_chain(void)
{
    struct Fixture fixture;
    struct Fixture expected;

    initialize_fixture(&fixture);
    expected = fixture;
    expected.tree_nodes[0][0].unknown00 = expected_keys[0];
    expected.tree_nodes[0][1].unknown00 = expected_keys[1];
    expected.tree_nodes[1][0].unknown00 = expected_keys[0];
    expected.tree_nodes[1][1].unknown00 = expected_keys[1];
    expected.tree_nodes[2][0].unknown00 = expected_keys[0];
    expected.tree_nodes[2][1].unknown00 = expected_keys[1];
    rebuild_count = 0;

    RefreshContextTreeKeysAt10(&fixture.context);
    RefreshContextTreeKeysAt14(&fixture.context);
    RefreshContextTreeKeysAt18(&fixture.context);

    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
    assert(rebuild_count == OWNER_COUNT);
    assert(rebuilt_owners[0] == &fixture.owners[0]);
    assert(rebuilt_owners[1] == &fixture.owners[1]);
    assert(rebuilt_owners[2] == &fixture.owners[2]);
}

static void test_empty_lists_still_rebuild_each_selected_owner(void)
{
    struct Fixture fixture;
    struct Fixture expected;

    initialize_fixture(&fixture);
    fixture.owners[0].list_head = 0;
    fixture.owners[1].list_head = 0;
    fixture.owners[2].list_head = 0;
    expected = fixture;
    rebuild_count = 0;

    RefreshContextTreeKeysAt10(&fixture.context);
    RefreshContextTreeKeysAt14(&fixture.context);
    RefreshContextTreeKeysAt18(&fixture.context);

    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
    assert(rebuild_count == OWNER_COUNT);
    assert(rebuilt_owners[0] == &fixture.owners[0]);
    assert(rebuilt_owners[1] == &fixture.owners[1]);
    assert(rebuilt_owners[2] == &fixture.owners[2]);
}

#endif

int main(void)
{
#ifdef CONTROLLED_CALLBACK
    test_wrapper_reloads_owner_after_callback();
#else
    test_callback_searches_and_sets_native_payload();
    test_callback_uses_first_in_order_payload_match();
    test_three_wrappers_compose_real_callback_chain();
    test_empty_lists_still_rebuild_each_selected_owner();
#endif
    return 0;
}
