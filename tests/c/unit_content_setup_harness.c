#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "content_record.h"
#include "inventory.h"
#include "unit.h"
#include "unit_content_setup.h"

enum EventKind {
    EVENT_PROPERTY = 1,
    EVENT_COPY,
    EVENT_BIT15,
    EVENT_BIT13,
    EVENT_READ_FIELD,
    EVENT_EQUALS94,
    EVENT_ADD,
    EVENT_ADJUST,
    EVENT_APPLY
};

struct Event {
    u8 kind;
    u8 arg;
    u16 item;
    u32 value;
};

struct Fixture {
    u32 beforeUnit;
    struct UnitFieldView unit;
    u32 afterUnit;
    struct ContentSourceEntry source;
    u32 afterSource;
};

struct Observable {
    struct Event events[32];
    u32 eventCount;
    u16 inventoryItems[5];
    u8 inventoryQuantities[5];
    u32 inventoryCount;
};

static struct Fixture *gFixture;
static struct Observable gObservable;
static u32 gPropertyResult;
static u8 gMutateByte7InProperty;
static u32 gControlledBit15;
static u32 gControlledBit13;
static u32 gControlledField3;
static u32 gControlledEquals94;
static u32 gMutationMode;

static void record_event(u8 kind, u8 arg, u16 item, u32 value)
{
    struct Event *event;
    assert(gObservable.eventCount < 32);
    event = &gObservable.events[gObservable.eventCount++];
    event->kind = kind;
    event->arg = arg;
    event->item = item;
    event->value = value;
}

static void copy_record(void *destination, const void *source, u32 size)
{
    assert(destination == (void *)((u8 *)&gFixture->unit + 0x2A));
    assert(source == (const void *)(gFixture->source.bytes + 8));
    record_event(EVENT_COPY, 0, 0, size);
    memcpy(destination, source, size);
}

void (*const gCopyRecordMemory)(void *, const void *, u32) = copy_record;

u32 GetUnitProperty(const u8 *unit, u8 property)
{
    assert(unit == (const u8 *)&gFixture->unit);
    record_event(EVENT_PROPERTY, property, 0, gPropertyResult);
    if (gMutateByte7InProperty)
        ((u8 *)unit)[7] = 69;
    return gPropertyResult;
}

int AddInventoryItem(u16 item, u8 quantity)
{
    u32 slot = gObservable.inventoryCount++;
    assert(slot < 5);
    record_event(EVENT_ADD, quantity, item, 0);
    gObservable.inventoryItems[slot] = item;
    gObservable.inventoryQuantities[slot] = quantity;
    if (gMutationMode && slot == 0)
        *(u16 *)((u8 *)&gFixture->unit + 0x2A) = 0x2202;
    return -17;
}

void AdjustInventoryEntryByte3(u16 item, u8 increase)
{
    record_event(EVENT_ADJUST, increase, item, 0);
    if (gMutationMode && gObservable.inventoryCount == 1)
        *(u16 *)((u8 *)&gFixture->unit + 0x2A) = 0x3303;
}

void ApplyItemAbilityFlags(u8 *unit, u16 item)
{
    assert(unit == (u8 *)&gFixture->unit);
    record_event(EVENT_APPLY, 0, item, 0);
}

#ifdef CONTROLLED_PREDICATES
u8 UnitField27HasBit15(const struct UnitFieldView *unit)
{
    assert(unit == &gFixture->unit);
    record_event(EVENT_BIT15, 0, 0, gControlledBit15);
    return (u8)gControlledBit15;
}

u8 UnitField27HasBit13(const struct UnitFieldView *unit)
{
    assert(unit == &gFixture->unit);
    record_event(EVENT_BIT13, 0, 0, gControlledBit13);
    return (u8)gControlledBit13;
}

u32 ReadUnitField(const struct UnitFieldView *unit, u8 field)
{
    assert(unit == &gFixture->unit);
    assert(field == 3);
    record_event(EVENT_READ_FIELD, field, 0, gControlledField3);
    return gControlledField3;
}

u8 UnitField01Equals(const struct UnitFieldView *unit, u8 value)
{
    assert(unit == &gFixture->unit);
    assert(value == 94);
    record_event(EVENT_EQUALS94, value, 0, gControlledEquals94);
    return (u8)gControlledEquals94;
}
#endif

static void reset_state(struct Fixture *fixture)
{
    memset(fixture, 0xA5, sizeof(*fixture));
    fixture->beforeUnit = 0xBADC0FFE;
    fixture->afterUnit = 0xC0DE1122;
    fixture->afterSource = 0xA11CE55A;
    memset(&gObservable, 0, sizeof(gObservable));
    gFixture = fixture;
    gPropertyResult = 0;
    gMutateByte7InProperty = 0;
    gControlledBit15 = 0;
    gControlledBit13 = 0;
    gControlledField3 = 0;
    gControlledEquals94 = 0;
    gMutationMode = 0;
}

static void set_source_items(struct Fixture *fixture,
                             const u16 items[5])
{
    unsigned i;
    for (i = 0; i < 5; i++) {
        fixture->source.bytes[8 + 2 * i] = (u8)items[i];
        fixture->source.bytes[9 + 2 * i] = (u8)(items[i] >> 8);
    }
}

static void add_expected_event(struct Observable *expected, u8 kind,
                               u8 arg, u16 item, u32 value)
{
    struct Event *event = &expected->events[expected->eventCount++];
    event->kind = kind;
    event->arg = arg;
    event->item = item;
    event->value = value;
}

static void expect_apply_only(struct Observable *expected,
                              const u16 items[5])
{
    unsigned i;
    for (i = 0; i < 5; i++)
        add_expected_event(expected, EVENT_APPLY, 0, items[i], 0);
}

static void expect_granted_items(struct Observable *expected,
                                 const u16 items[5])
{
    unsigned i;
    for (i = 0; i < 5; i++) {
        add_expected_event(expected, EVENT_ADD, 1, items[i], 0);
        add_expected_event(expected, EVENT_ADJUST, 1, items[i], 0);
        add_expected_event(expected, EVENT_APPLY, 0, items[i], 0);
        expected->inventoryItems[i] = items[i];
        expected->inventoryQuantities[i] = 1;
        expected->inventoryCount++;
    }
}

static void check_property_writer(void)
{
    static const u8 initialFlags[] = {0, 67, 68, 69, 70, 255};
    static const u8 expectedFlag[] = {0, 0, 1, 1, 0, 0};
    unsigned i;

    for (i = 0; i < sizeof(initialFlags); i++) {
        struct Fixture fixture, expected;
        struct Observable expectedTrace;
        reset_state(&fixture);
        fixture.unit.bytes04[3] = initialFlags[i];
        fixture.unit.half18[8] = 0x5143;
        gPropertyResult = 0x13;
        expected = fixture;
        expected.unit.bytes04[3] = initialFlags[i];
        *((u8 *)&expected.unit + 0x53) = 0xE4;
        if (expectedFlag[i]) expected.unit.half18[8] |= 0x0800;
        memset(&expectedTrace, 0, sizeof(expectedTrace));
        add_expected_event(&expectedTrace, EVENT_PROPERTY, 33, 0, 0x13);

        ApplyUnitContentProperty33((u8 *)&fixture.unit);

        assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
        assert(memcmp(&gObservable, &expectedTrace,
                      sizeof(expectedTrace)) == 0);
    }

    {
        struct Fixture fixture, expected;
        struct Observable expectedTrace;
        reset_state(&fixture);
        fixture.unit.bytes04[3] = 67;
        fixture.unit.half18[8] = 0xA123;
        gPropertyResult = 0;
        gMutateByte7InProperty = 1;
        expected = fixture;
        expected.unit.bytes04[3] = 69;
        expected.unit.half18[8] |= 0x0800;
        memset(&expectedTrace, 0, sizeof(expectedTrace));
        add_expected_event(&expectedTrace, EVENT_PROPERTY, 33, 0, 0);

        ApplyUnitContentProperty33((u8 *)&fixture.unit);

        assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
        assert(memcmp(&gObservable, &expectedTrace,
                      sizeof(expectedTrace)) == 0);
    }
}

#ifndef CONTROLLED_PREDICATES
static void check_real_predicate_composition(void)
{
    static const u16 items[5] = {0x1101, 0, 0x2202, 0, 0x3303};
    struct Fixture fixture, expected;
    struct Observable expectedTrace;

    /* Set bit15 denies grants; actual bit13 and field predicates are clear. */
    reset_state(&fixture);
    set_source_items(&fixture, items);
    fixture.unit.half18[8] = 0x8000;
    expected = fixture;
    memcpy((u8 *)&expected.unit + 0x2A, fixture.source.bytes + 8, 10);
    memset(&expectedTrace, 0, sizeof(expectedTrace));
    add_expected_event(&expectedTrace, EVENT_COPY, 0, 0, 10);
    expect_apply_only(&expectedTrace, items);
    LoadUnitContentEquipment((u8 *)&fixture.unit, &fixture.source);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
    assert(memcmp(&gObservable, &expectedTrace, sizeof(expectedTrace)) == 0);

    /* Bit13 also denies grants when bit15 is clear. */
    reset_state(&fixture);
    set_source_items(&fixture, items);
    fixture.unit.half18[8] = 0x2000;
    expected = fixture;
    memcpy((u8 *)&expected.unit + 0x2A, fixture.source.bytes + 8, 10);
    memset(&expectedTrace, 0, sizeof(expectedTrace));
    add_expected_event(&expectedTrace, EVENT_COPY, 0, 0, 10);
    expect_apply_only(&expectedTrace, items);
    LoadUnitContentEquipment((u8 *)&fixture.unit, &fixture.source);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
    assert(memcmp(&gObservable, &expectedTrace, sizeof(expectedTrace)) == 0);

    /* Selector3==23 plus field01==94 preserves grant; item zero is called. */
    reset_state(&fixture);
    set_source_items(&fixture, items);
    fixture.unit.bytes04[2] = 23;
    fixture.unit.bytes04[0] = 94;
    fixture.unit.half18[8] = 0;
    expected = fixture;
    memcpy((u8 *)&expected.unit + 0x2A, fixture.source.bytes + 8, 10);
    memset(&expectedTrace, 0, sizeof(expectedTrace));
    add_expected_event(&expectedTrace, EVENT_COPY, 0, 0, 10);
    expect_granted_items(&expectedTrace, items);
    LoadUnitContentEquipment((u8 *)&fixture.unit, &fixture.source);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
    assert(memcmp(&gObservable, &expectedTrace, sizeof(expectedTrace)) == 0);

    /* Selector3==23 and field01!=94 denies grant. */
    reset_state(&fixture);
    set_source_items(&fixture, items);
    fixture.unit.bytes04[2] = 23;
    fixture.unit.bytes04[0] = 93;
    fixture.unit.half18[8] = 0;
    expected = fixture;
    memcpy((u8 *)&expected.unit + 0x2A, fixture.source.bytes + 8, 10);
    memset(&expectedTrace, 0, sizeof(expectedTrace));
    add_expected_event(&expectedTrace, EVENT_COPY, 0, 0, 10);
    expect_apply_only(&expectedTrace, items);
    LoadUnitContentEquipment((u8 *)&fixture.unit, &fixture.source);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
    assert(memcmp(&gObservable, &expectedTrace, sizeof(expectedTrace)) == 0);

    /* A non-23 selector grants regardless of field01. */
    reset_state(&fixture);
    set_source_items(&fixture, items);
    fixture.unit.bytes04[2] = 22;
    fixture.unit.bytes04[0] = 93;
    fixture.unit.half18[8] = 0;
    expected = fixture;
    memcpy((u8 *)&expected.unit + 0x2A, fixture.source.bytes + 8, 10);
    memset(&expectedTrace, 0, sizeof(expectedTrace));
    add_expected_event(&expectedTrace, EVENT_COPY, 0, 0, 10);
    expect_granted_items(&expectedTrace, items);
    LoadUnitContentEquipment((u8 *)&fixture.unit, &fixture.source);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
    assert(memcmp(&gObservable, &expectedTrace, sizeof(expectedTrace)) == 0);
}
#else
static void check_controlled_order_and_reload(void)
{
    static const u16 items[5] = {0x1101, 0, 0x2202, 0, 0x3303};
    struct Fixture fixture, expected;
    struct Observable expectedTrace;

    /* A denied bit15 still runs selector3 and conditional equality94; bit13
     * is skipped. Equality is observable even though grant was already off. */
    reset_state(&fixture);
    set_source_items(&fixture, items);
    gControlledBit15 = 1;
    gControlledField3 = 23;
    gControlledEquals94 = 0;
    expected = fixture;
    memcpy((u8 *)&expected.unit + 0x2A, fixture.source.bytes + 8, 10);
    memset(&expectedTrace, 0, sizeof(expectedTrace));
    add_expected_event(&expectedTrace, EVENT_COPY, 0, 0, 10);
    add_expected_event(&expectedTrace, EVENT_BIT15, 0, 0, 1);
    add_expected_event(&expectedTrace, EVENT_READ_FIELD, 3, 0, 23);
    add_expected_event(&expectedTrace, EVENT_EQUALS94, 94, 0, 0);
    expect_apply_only(&expectedTrace, items);
    LoadUnitContentEquipment((u8 *)&fixture.unit, &fixture.source);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
    assert(memcmp(&gObservable, &expectedTrace, sizeof(expectedTrace)) == 0);

    /* Bit13 runs only after bit15 clears; selector3 still runs and equality
     * stays skipped when selector3 is not 23. */
    reset_state(&fixture);
    set_source_items(&fixture, items);
    gControlledBit13 = 1;
    gControlledField3 = 22;
    expected = fixture;
    memcpy((u8 *)&expected.unit + 0x2A, fixture.source.bytes + 8, 10);
    memset(&expectedTrace, 0, sizeof(expectedTrace));
    add_expected_event(&expectedTrace, EVENT_COPY, 0, 0, 10);
    add_expected_event(&expectedTrace, EVENT_BIT15, 0, 0, 0);
    add_expected_event(&expectedTrace, EVENT_BIT13, 0, 0, 1);
    add_expected_event(&expectedTrace, EVENT_READ_FIELD, 3, 0, 22);
    expect_apply_only(&expectedTrace, items);
    LoadUnitContentEquipment((u8 *)&fixture.unit, &fixture.source);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
    assert(memcmp(&gObservable, &expectedTrace, sizeof(expectedTrace)) == 0);

    /* Exact94 and only exact94 permits this final grant condition. */
    reset_state(&fixture);
    set_source_items(&fixture, items);
    gControlledField3 = 23;
    gControlledEquals94 = 1;
    expected = fixture;
    memcpy((u8 *)&expected.unit + 0x2A, fixture.source.bytes + 8, 10);
    memset(&expectedTrace, 0, sizeof(expectedTrace));
    add_expected_event(&expectedTrace, EVENT_COPY, 0, 0, 10);
    add_expected_event(&expectedTrace, EVENT_BIT15, 0, 0, 0);
    add_expected_event(&expectedTrace, EVENT_BIT13, 0, 0, 0);
    add_expected_event(&expectedTrace, EVENT_READ_FIELD, 3, 0, 23);
    add_expected_event(&expectedTrace, EVENT_EQUALS94, 94, 0, 1);
    expect_granted_items(&expectedTrace, items);
    LoadUnitContentEquipment((u8 *)&fixture.unit, &fixture.source);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
    assert(memcmp(&gObservable, &expectedTrace, sizeof(expectedTrace)) == 0);

    reset_state(&fixture);
    set_source_items(&fixture, items);
    gControlledField3 = 23;
    gControlledEquals94 = 1;
    gMutationMode = 1;
    expected = fixture;
    memcpy((u8 *)&expected.unit + 0x2A, fixture.source.bytes + 8, 10);
    *(u16 *)((u8 *)&expected.unit + 0x2A) = 0x3303;
    memset(&expectedTrace, 0, sizeof(expectedTrace));
    add_expected_event(&expectedTrace, EVENT_COPY, 0, 0, 10);
    add_expected_event(&expectedTrace, EVENT_BIT15, 0, 0, 0);
    add_expected_event(&expectedTrace, EVENT_BIT13, 0, 0, 0);
    add_expected_event(&expectedTrace, EVENT_READ_FIELD, 3, 0, 23);
    add_expected_event(&expectedTrace, EVENT_EQUALS94, 94, 0, 1);
    expect_granted_items(&expectedTrace, items);
    expectedTrace.events[5].item = 0x1101;
    expectedTrace.events[6].item = 0x2202;
    expectedTrace.events[7].item = 0x3303;
    expectedTrace.inventoryItems[0] = 0x1101;
    assert(memcmp(&fixture.source, &expected.source,
                  sizeof(fixture.source)) == 0);
    LoadUnitContentEquipment((u8 *)&fixture.unit, &fixture.source);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
    assert(memcmp(&gObservable, &expectedTrace, sizeof(expectedTrace)) == 0);
}
#endif

int main(void)
{
    struct Fixture fixture;
    assert(sizeof(struct ContentSourceEntry) == 48);
    assert(sizeof(struct UnitFieldView) == 0xFC);
    assert(offsetof(struct UnitFieldView, half18) + 8 * sizeof(u16) == 0x28);
    assert(offsetof(struct UnitFieldView, bytes04) + 3 == 7);

    check_property_writer();
    reset_state(&fixture);
#ifndef CONTROLLED_PREDICATES
    check_real_predicate_composition();
#else
    check_controlled_order_and_reload();
#endif
    return 0;
}
