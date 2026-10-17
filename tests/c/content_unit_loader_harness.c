#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "content_unit_loader.h"

#define SOURCE_COUNT 3
#define UNIT_COUNT 4

struct Fixture {
    u32 before_guard;
    struct ContentRecord record;
    u8 record_guard[8];
    struct ContentSourceEntry source[SOURCE_COUNT];
    u32 source_guard;
    struct UnitRecord units[UNIT_COUNT];
    u32 unit_guard;
};

static struct Fixture *sFixture;
static unsigned sMode;
static unsigned sCalls;
static struct UnitRecord *sTargets[4];
static const struct ContentSourceEntry *sSources[4];
static u32 sSelectors[4];

void InitializeUnitFromContentEntry(struct UnitRecord *destination,
                                    const struct ContentSourceEntry *source_entry,
                                    u32 selector)
{
    unsigned call = sCalls++;

    assert(call < 4);
    sTargets[call] = destination;
    sSources[call] = source_entry;
    sSelectors[call] = selector;
    destination->fields.bytes04[1] = source_entry->bytes[0];
    if (call == 1)
        destination->fields.bytes04[2] = source_entry->bytes[47];

    if (sMode == 1) {
        destination->fields.bytes04[0] = 1;
        if (call == 0)
            sFixture->record.sourceCount = 3;
    } else if (sMode == 2) {
        destination->fields.bytes04[0] = 1;
        sFixture->record.sourceCount = 1;
    }
}

static void initialize(struct Fixture *fixture, u16 source_count)
{
    unsigned i;

    memset(fixture, 0xA5, sizeof(*fixture));
    fixture->before_guard = 0x12345678;
    fixture->record.sourceCount = source_count;
    fixture->record.unknown02 = 0x6b4d;
    fixture->record.sourceEntries = &fixture->source[0];
    memset(fixture->record.unknown08, 0x39, sizeof(fixture->record.unknown08));
    fixture->record.values = 0;
    memset(fixture->record.unknown20, 0x71, sizeof(fixture->record.unknown20));
    fixture->source_guard = 0x87654321;
    fixture->unit_guard = 0x0badcafe;
    for (i = 0; i < SOURCE_COUNT; i++) {
        memset(&fixture->source[i], 0x20 + i, sizeof(fixture->source[i]));
        fixture->source[i].bytes[0] = (u8)(0x31 + i);
        fixture->source[i].bytes[47] = (u8)(0x71 + i);
    }
    for (i = 0; i < UNIT_COUNT; i++) {
        memset(&fixture->units[i], 0, sizeof(fixture->units[i]));
        fixture->units[i].fields.bytes04[0] = i == 0 ? 1 : 0;
        fixture->units[i].fields.word00 = 0x11220000u + i;
    }
    memset(sTargets, 0, sizeof(sTargets));
    memset(sSources, 0, sizeof(sSources));
    memset(sSelectors, 0xff, sizeof(sSelectors));
    sFixture = fixture;
    sMode = 0;
    sCalls = 0;
}

static void assert_layout(void)
{
    assert(sizeof(((struct ContentRecord *)0)->unknown08) == 20);
    assert(offsetof(struct ContentRecord, sourceCount) == 0);
    if (sizeof(void *) == 4) {
        assert(offsetof(struct ContentRecord, sourceEntries) == 4);
        assert(offsetof(struct ContentRecord, values) == 28);
        assert(sizeof(struct ContentRecord) == 40);
    } else {
        assert(offsetof(struct ContentRecord, sourceEntries) == 8);
        assert(offsetof(struct ContentRecord, values) == 40);
        assert(offsetof(struct ContentRecord, unknown20) == 48);
        assert(sizeof(struct ContentRecord) == 56);
    }
    assert(sizeof(struct UnitRecord) == 264);
    assert(sizeof(struct ContentSourceEntry) == 48);
    assert(offsetof(struct UnitRecord, fields) == 0);
    assert(offsetof(struct UnitFieldView, bytes04) + 0 == 4);
}

static void check_signed_limit_and_empty_source(void)
{
    struct Fixture fixture;
    struct Fixture expected;

    initialize(&fixture, 3);
    expected = fixture;
    assert(LoadContentUnitRecords(&fixture.record, fixture.units, -1) == 0);
    assert(sCalls == 0);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);

    assert(LoadContentUnitRecords(&fixture.record, fixture.units, 0) == 0);
    assert(sCalls == 0);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);

    initialize(&fixture, 0);
    expected = fixture;
    assert(LoadContentUnitRecords(&fixture.record, fixture.units, 2) == 0);
    assert(sCalls == 0);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
}

static void check_cursor_reuses_free_destination(void)
{
    struct Fixture fixture;
    struct Fixture expected;
    int result;

    initialize(&fixture, 3);
    expected = fixture;
    expected.units[1].fields.bytes04[1] = 0x32;
    expected.units[1].fields.bytes04[2] = 0x72;
    sFixture = &fixture;
    result = LoadContentUnitRecords(&fixture.record, fixture.units, 2);

    assert(result == 2);
    assert(sCalls == 2);
    assert(sTargets[0] == &fixture.units[1]);
    assert(sTargets[1] == &fixture.units[1]);
    assert(sSources[0] == &fixture.source[0]);
    assert(sSources[1] == &fixture.source[1]);
    assert(sTargets[1] - sTargets[0] == 0);
    assert((const u8 *)sSources[1] - (const u8 *)sSources[0] == 48);
    assert(sSelectors[0] == 0 && sSelectors[1] == 0);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
}

static void check_callback_occupancy_advances_cursor(void)
{
    struct Fixture fixture;
    struct Fixture expected;
    int result;

    initialize(&fixture, 1);
    expected = fixture;
    expected.record.sourceCount = 3;
    expected.units[1].fields.bytes04[0] = 1;
    expected.units[1].fields.bytes04[1] = 0x31;
    expected.units[2].fields.bytes04[0] = 1;
    expected.units[2].fields.bytes04[1] = 0x32;
    expected.units[2].fields.bytes04[2] = 0x72;
    expected.units[3].fields.bytes04[0] = 1;
    expected.units[3].fields.bytes04[1] = 0x33;
    sFixture = &fixture;
    sMode = 1;
    result = LoadContentUnitRecords(&fixture.record, fixture.units, 3);

    assert(result == 3);
    assert(fixture.record.sourceCount == 3);
    assert(sCalls == 3);
    assert(sTargets[0] == &fixture.units[1]);
    assert(sTargets[1] == &fixture.units[2]);
    assert(sTargets[2] == &fixture.units[3]);
    assert(sTargets[1] - sTargets[0] == 1);
    assert(sSources[0] == &fixture.source[0]);
    assert(sSources[1] == &fixture.source[1]);
    assert(sSources[2] == &fixture.source[2]);
    assert((const u8 *)sSources[1] - (const u8 *)sSources[0] == 48);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
}

static void check_record_count_is_reloaded_after_callback(void)
{
    struct Fixture fixture;
    struct Fixture expected;
    int result;

    initialize(&fixture, 2);
    expected = fixture;
    expected.units[1].fields.bytes04[0] = 1;
    expected.units[1].fields.bytes04[1] = 0x31;
    expected.record.sourceCount = 1;
    sFixture = &fixture;
    sMode = 2;
    result = LoadContentUnitRecords(&fixture.record, fixture.units, 2);

    assert(result == 1);
    assert(sCalls == 1);
    assert(sTargets[0] == &fixture.units[1]);
    assert(sSources[0] == &fixture.source[0]);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
}

int main(void)
{
    assert_layout();
    check_signed_limit_and_empty_source();
    check_cursor_reuses_free_destination();
    check_callback_occupancy_advances_cursor();
    check_record_count_is_reloaded_after_callback();
    return 0;
}
