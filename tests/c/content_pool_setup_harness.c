#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "content_pool_setup.h"
#include "content_unit_loader.h"

#define UNIT_COUNT 12
#define RECORD_COUNT 4

const struct ContentRecord gContentRecords[RECORD_COUNT] = {{0}};
struct UnitRecord gUnitPoolA[UNIT_COUNT];
struct UnitRecord gUnitPoolB[24];

struct CallbackImage {
    unsigned loader_calls;
    unsigned resolver_calls;
    const struct ContentRecord *loader_record;
    struct UnitRecord *loader_pool;
    int loader_limit;
    struct ListCountContext *contexts[UNIT_COUNT];
    struct UnitRecord *units[UNIT_COUNT];
    struct ContentConsumerEntry *results[UNIT_COUNT];
    unsigned order_count;
    unsigned order[UNIT_COUNT + 1];
};

struct WholeImage {
    struct ContentRecord records[RECORD_COUNT];
    struct UnitRecord pool[UNIT_COUNT];
    struct UnitRecord other_pool[24];
    struct UnitFieldView objects[UNIT_COUNT];
    struct ContentConsumerEntry entries[UNIT_COUNT];
    struct ListCountContext context;
};

static struct UnitFieldView sObjects[UNIT_COUNT];
static struct ContentConsumerEntry sEntries[UNIT_COUNT];
static struct ListCountContext sContext;
static struct UnitRecord sLoaderImage[UNIT_COUNT];
static struct CallbackImage sCallbacks;
static unsigned sEntryForUnit[UNIT_COUNT];
static unsigned sMutateLaterSlot;
static unsigned sMutateLaterTo;
static int sMutateLater;

int LoadContentUnitRecords(const struct ContentRecord *record,
                           struct UnitRecord *destination,
                           signed int limit)
{
    sCallbacks.loader_calls++;
    sCallbacks.order[sCallbacks.order_count++] = 0;
    sCallbacks.loader_record = record;
    sCallbacks.loader_pool = destination;
    sCallbacks.loader_limit = limit;
    memcpy(destination, sLoaderImage, sizeof(sLoaderImage));
    return -71;
}

struct ContentConsumerEntry *ResolveOrCreateContentUnitEntry(
    struct ListCountContext *context, struct UnitRecord *unit)
{
    unsigned slot;
    unsigned result_index;
    unsigned call = sCallbacks.resolver_calls++;
    struct ContentConsumerEntry *result;

    assert(sCallbacks.loader_calls == 1);
    assert(call < UNIT_COUNT);
    for (slot = 0; slot < UNIT_COUNT; slot++)
        if (unit == &gUnitPoolA[slot]) break;
    assert(slot < UNIT_COUNT);
    result_index = sEntryForUnit[slot];
    assert(result_index < UNIT_COUNT);
    result = &sEntries[result_index];
    sCallbacks.contexts[call] = context;
    sCallbacks.units[call] = unit;
    sCallbacks.results[call] = result;
    sCallbacks.order[sCallbacks.order_count++] = slot + 1;
    if (sMutateLater && slot == sMutateLaterSlot)
        gUnitPoolA[sMutateLaterSlot + 1].fields.bytes04[0] =
            (u8)sMutateLaterTo;
    return result;
}

static void snapshot(struct WholeImage *image)
{
    memset(image, 0, sizeof(*image));
    memcpy(image->records, gContentRecords, sizeof(image->records));
    memcpy(image->pool, gUnitPoolA, sizeof(image->pool));
    memcpy(image->other_pool, gUnitPoolB, sizeof(image->other_pool));
    memcpy(image->objects, sObjects, sizeof(image->objects));
    memcpy(image->entries, sEntries, sizeof(image->entries));
    image->context = sContext;
}

static void initialize(void)
{
    unsigned i;
    memset(&sCallbacks, 0, sizeof(sCallbacks));
    memset(&sContext, 0, sizeof(sContext));
    memset(sContext.unknown00, 0x4c, sizeof(sContext.unknown00));
    memset(sObjects, 0x31, sizeof(sObjects));
    memset(sEntries, 0x52, sizeof(sEntries));
    memset(gUnitPoolA, 0x73, sizeof(gUnitPoolA));
    memset(sLoaderImage, 0x73, sizeof(sLoaderImage));
    for (i = 0; i < UNIT_COUNT; i++) {
        sLoaderImage[i].fields.bytes04[0] = 0;
        sObjects[i].bytesF1[5] = (u8)(0x20 + i);
        sObjects[i].bytesF1[6] = (u8)(0x80 + i);
        sObjects[i].bytesF1[8] = (u8)(0xa0 + i);
        sObjects[i].bytesF1[9] = (u8)(0xc0 + i);
        sEntries[i].object = &sObjects[i];
        sEntryForUnit[i] = i;
    }
    sMutateLater = 0;
    sMutateLaterSlot = 0;
    sMutateLaterTo = 0;
}

static void expect_occupied(unsigned slot, u8 value)
{
    sLoaderImage[slot].fields.bytes04[0] = value;
}

static void assert_image_equal(const struct WholeImage *expected)
{
    struct WholeImage actual;
    snapshot(&actual);
    assert(memcmp(actual.records, expected->records, sizeof(actual.records)) == 0);
    assert(memcmp(actual.pool, expected->pool, sizeof(actual.pool)) == 0);
    assert(memcmp(actual.other_pool, expected->other_pool,
                  sizeof(actual.other_pool)) == 0);
    assert(memcmp(actual.objects, expected->objects, sizeof(actual.objects)) == 0);
    assert(memcmp(actual.entries, expected->entries, sizeof(actual.entries)) == 0);
    assert(memcmp(&actual.context, &expected->context,
                  sizeof(actual.context)) == 0);
}

static void assert_loader(u16 record_index)
{
    assert(sCallbacks.loader_calls == 1);
    assert(sCallbacks.loader_record == &gContentRecords[record_index]);
    assert(sCallbacks.loader_pool == gUnitPoolA);
    assert(sCallbacks.loader_limit == 12);
}

static void assert_no_resolver_calls(void)
{
    assert(sCallbacks.resolver_calls == 0);
    assert(sCallbacks.order_count == 1);
    assert(sCallbacks.order[0] == 0);
}

static void test_empty_pool_and_index_truncation(void)
{
    struct WholeImage expected;

    initialize();
    snapshot(&expected);
    LoadAndPrepareContentUnitPool(0x10002u, &sContext);
    memcpy(expected.pool, sLoaderImage, sizeof(expected.pool));
    assert_image_equal(&expected);
    assert_loader(2);
    assert_no_resolver_calls();
}

static void test_sparse_slots_and_distinct_returned_objects(void)
{
    struct WholeImage expected;
    struct WholeImage initial;
    static const unsigned slots[] = {1, 4, 10};
    static const u8 occupancy[] = {2, 0x80, 7};
    static const unsigned result_objects[] = {9, 2, 6};
    unsigned i;

    initialize();
    expect_occupied(slots[0], occupancy[0]);
    expect_occupied(slots[1], occupancy[1]);
    expect_occupied(slots[2], occupancy[2]);
    sEntryForUnit[slots[0]] = 9;
    sEntryForUnit[slots[1]] = 2;
    sEntryForUnit[slots[2]] = 6;
    snapshot(&initial);
    LoadAndPrepareContentUnitPool(3, &sContext);

    expected = initial;
    memcpy(expected.pool, sLoaderImage, sizeof(expected.pool));
    expected.objects[9].bytesF1[8] = 0x29;
    expected.objects[9].bytesF1[9] = 0x89;
    expected.objects[2].bytesF1[8] = 0x22;
    expected.objects[2].bytesF1[9] = 0x82;
    expected.objects[6].bytesF1[8] = 0x26;
    expected.objects[6].bytesF1[9] = 0x86;
    assert_image_equal(&expected);
    assert_loader(3);
    assert(sCallbacks.resolver_calls == 3);
    assert(sCallbacks.order_count == 4);
    assert(sCallbacks.order[0] == 0);
    for (i = 0; i < 3; i++) {
        unsigned call = i;
        assert(sCallbacks.order[call + 1] == slots[i] + 1);
        assert(sCallbacks.contexts[call] == &sContext);
        assert(sCallbacks.units[call] == &gUnitPoolA[slots[i]]);
        assert(sCallbacks.results[call] == &sEntries[result_objects[i]]);
    }
}

static void test_twelve_slots_and_non_one_occupancy(void)
{
    struct WholeImage expected;
    struct WholeImage initial;
    unsigned i;

    initialize();
    for (i = 0; i < UNIT_COUNT; i++)
        expect_occupied(i, (u8)(i + 1));
    /* A permutation makes every resolver result's nested object distinct. */
    for (i = 0; i < UNIT_COUNT; i++)
        sEntryForUnit[i] = (i * 5) % UNIT_COUNT;
    snapshot(&initial);
    LoadAndPrepareContentUnitPool(1, &sContext);

    expected = initial;
    memcpy(expected.pool, sLoaderImage, sizeof(expected.pool));
    for (i = 0; i < UNIT_COUNT; i++) {
        unsigned object_index = (i * 5) % UNIT_COUNT;
        expected.objects[object_index].bytesF1[8] = (u8)(0x20 + object_index);
        expected.objects[object_index].bytesF1[9] = (u8)(0x80 + object_index);
    }
    assert_image_equal(&expected);
    assert_loader(1);
    assert(sCallbacks.resolver_calls == UNIT_COUNT);
    assert(sCallbacks.order_count == UNIT_COUNT + 1);
    for (i = 0; i < UNIT_COUNT; i++) {
        assert(sCallbacks.order[i + 1] == i + 1);
        assert(sCallbacks.contexts[i] == &sContext);
        assert(sCallbacks.units[i] == &gUnitPoolA[i]);
        assert(sCallbacks.results[i] == &sEntries[(i * 5) % UNIT_COUNT]);
    }
}

static void test_later_slot_occupancy_is_reloaded(void)
{
    struct WholeImage expected;
    struct WholeImage initial;

    initialize();
    expect_occupied(0, 1);
    sEntryForUnit[0] = 7;
    sEntryForUnit[1] = 3;
    sMutateLater = 1;
    sMutateLaterSlot = 0;
    sMutateLaterTo = 0x55;
    snapshot(&initial);
    LoadAndPrepareContentUnitPool(0, &sContext);

    expected = initial;
    memcpy(expected.pool, sLoaderImage, sizeof(expected.pool));
    expected.pool[1].fields.bytes04[0] = 0x55;
    expected.objects[7].bytesF1[8] = 0x27;
    expected.objects[7].bytesF1[9] = 0x87;
    expected.objects[3].bytesF1[8] = 0x23;
    expected.objects[3].bytesF1[9] = 0x83;
    assert_image_equal(&expected);
    assert_loader(0);
    assert(sCallbacks.resolver_calls == 2);
    assert(sCallbacks.order_count == 3);
    assert(sCallbacks.order[0] == 0);
    assert(sCallbacks.order[1] == 1);
    assert(sCallbacks.order[2] == 2);
    assert(sCallbacks.units[0] == &gUnitPoolA[0]);
    assert(sCallbacks.units[1] == &gUnitPoolA[1]);
    assert(sCallbacks.results[0] == &sEntries[7]);
    assert(sCallbacks.results[1] == &sEntries[3]);
}

int main(void)
{
    assert(sizeof(gContentRecords) / sizeof(gContentRecords[0]) == RECORD_COUNT);
    assert(sizeof(struct UnitRecord) == 264);
    assert(offsetof(struct UnitFieldView, bytes04) == 4);
    assert(offsetof(struct UnitFieldView, bytesF1) == 0xf1);
    assert(offsetof(struct ContentConsumerEntry, object) == 0);
    test_empty_pool_and_index_truncation();
    test_sparse_slots_and_distinct_returned_objects();
    test_twelve_slots_and_non_one_occupancy();
    test_later_slot_occupancy_is_reloaded();
    return 0;
}
