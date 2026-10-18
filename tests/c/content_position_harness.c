#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "content_position.h"

struct PositionFixture {
    u8 before[8];
    struct ContentConsumerEntry entry;
    struct UnitFieldView units[2];
    u8 after[8];
};

static struct ContentConsumerEntry *sEntryToMutate;
static struct UnitFieldView *sReplacementObject;
static struct UnitFieldView *sObservedObject;
static signed int sObservedX;
static signed int sObservedY;
static u32 sStubReturn;

typedef char ContentPositionHostLayout[
    sizeof(void *) != 8 ||
    (offsetof(struct ContentConsumerEntry, position08) == 0x0C &&
     offsetof(struct ContentConsumerEntry, coordinateValue0A) == 0x0E &&
     offsetof(struct ContentConsumerEntry, position0C) == 0x10 &&
     offsetof(struct ContentConsumerEntry, unknown24) == 0x28 &&
     offsetof(struct ContentConsumerEntry, value5c) == 0x60 &&
     sizeof(struct ContentConsumerEntry) == 0x68) ? 1 : -1];
typedef char ContentPositionUnitByteOffsets[
    offsetof(struct UnitFieldView, bytesF1) + 5 == 0xF6 &&
    offsetof(struct UnitFieldView, bytesF1) + 6 == 0xF7 &&
    sizeof(struct UnitFieldView) == 0xFC ? 1 : -1];

/* Controlled stand-in for the copied ROM dependency at 0x131BE0. */
u32 GetContentCoordinateQueryValue(struct UnitFieldView *object,
                                   signed int x, signed int y)
{
    sObservedObject = object;
    sObservedX = x;
    sObservedY = y;
    if (sEntryToMutate != 0)
        sEntryToMutate->object = sReplacementObject;
    return sStubReturn;
}

static void init_fixture(struct PositionFixture *fixture)
{
    memset(fixture, 0xA5, sizeof(*fixture));
    fixture->entry.object = &fixture->units[0];
    fixture->entry.unknown04 = 0x12345678;
    fixture->entry.position08 = (signed short)0x1357;
    fixture->entry.coordinateValue0A = 0x2468;
    fixture->entry.position0C = (signed short)0x369C;
    fixture->entry.value5c = 0xBEEF;
    fixture->units[0].bytesF1[5] = 0x31;
    fixture->units[0].bytesF1[6] = 0x42;
    fixture->units[1].bytesF1[5] = 0x53;
    fixture->units[1].bytesF1[6] = 0x64;
}

static void test_forwarder_sign_extends_and_preserves_u32(void)
{
    struct PositionFixture fixture;
    struct PositionFixture before;
    u32 result;

    init_fixture(&fixture);
    before = fixture;
    sEntryToMutate = 0;
    sReplacementObject = 0;
    sStubReturn = 0xF1234567;
    sObservedObject = 0;
    result = ReadContentUnitCoordinateValue(&fixture.entry,
                                            (signed int)0x12345680,
                                            (signed int)0x7654327F);

    assert(result == 0xF1234567);
    assert(sObservedObject == &fixture.units[0]);
    assert(sObservedX == -128);
    assert(sObservedY == 127);
    assert(memcmp(&fixture, &before, sizeof(fixture)) == 0);
}

static void test_setter_whole_image_and_mutated_object(void)
{
    struct PositionFixture actual;
    struct PositionFixture expected;

    init_fixture(&actual);
    expected = actual;
    sEntryToMutate = &actual.entry;
    sReplacementObject = &actual.units[1];
    sStubReturn = 0xFEDCBA98;

    /* Fixed expected whole-image updates from the observed 0x99E68 stores. */
    expected.entry.position08 = (signed short)-4080;
    expected.entry.coordinateValue0A = 0xA980;
    expected.entry.position0C = (signed short)4080;
    expected.entry.object = &actual.units[1];
    expected.units[1].bytesF1[5] = 0x80;
    expected.units[1].bytesF1[6] = 0x7F;
    SetContentEntryPosition(&actual.entry,
                            (signed int)0x12345680,
                            (signed int)0x7654327F);

    assert(sObservedObject == &actual.units[0]);
    assert(sObservedX == -128);
    assert(sObservedY == 127);
    /* Compare every live field, including both units and outer guards. */
    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
}

static void test_low_byte_aliases_and_unsigned_return_wrap(void)
{
    struct PositionFixture actual;
    struct PositionFixture expected;

    init_fixture(&actual);
    expected = actual;
    sEntryToMutate = 0;
    sReplacementObject = 0;
    sStubReturn = 0xFFFFFFFF;

    expected.entry.position08 = (signed short)-16;
    expected.entry.coordinateValue0A = 0xFFF0;
    expected.entry.position0C = (signed short)-4080;
    expected.units[0].bytesF1[5] = 0xFF;
    expected.units[0].bytesF1[6] = 0x80;
    SetContentEntryPosition(&actual.entry, -1, 0x80);

    assert(sObservedObject == &actual.units[0]);
    assert(sObservedX == -1);
    assert(sObservedY == -128);
    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
}

static void test_zero_coordinates_keep_fixed_sixteens(void)
{
    struct PositionFixture actual;
    struct PositionFixture expected;

    init_fixture(&actual);
    expected = actual;
    sEntryToMutate = 0;
    sReplacementObject = 0;
    sStubReturn = 0x10;
    expected.entry.position08 = 16;
    expected.entry.coordinateValue0A = 0x0100;
    expected.entry.position0C = 16;
    expected.units[0].bytesF1[5] = 0;
    expected.units[0].bytesF1[6] = 0;
    SetContentEntryPosition(&actual.entry, 0, 0);

    assert(sObservedObject == &actual.units[0]);
    assert(sObservedX == 0);
    assert(sObservedY == 0);
    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
}

int main(void)
{
    assert(offsetof(struct ContentConsumerEntry, position08) == 8 ||
           sizeof(void *) != 4);
    assert(offsetof(struct ContentConsumerEntry, coordinateValue0A) == 10 ||
           sizeof(void *) != 4);
    assert(offsetof(struct ContentConsumerEntry, position0C) == 12 ||
           sizeof(void *) != 4);
    assert(sizeof(struct UnitFieldView) == 0xFC);
    test_forwarder_sign_extends_and_preserves_u32();
    test_setter_whole_image_and_mutated_object();
    test_low_byte_aliases_and_unsigned_return_wrap();
    test_zero_coordinates_keep_fixed_sixteens();
    return 0;
}
