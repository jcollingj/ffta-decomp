#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "content_unit_select.h"
#include "unit.h"

struct SelectionContext {
    struct ContentConsumerEntry *selected;
    u32 guard;
};

struct Fixture {
    struct ContentConsumerEntry entries[2];
    struct UnitFieldView units[2];
    struct SelectionContext context;
};

typedef int (*ContentEntryCallback)(void *entry, void *context);
typedef char EntryObjectAtZero[
    offsetof(struct ContentConsumerEntry, object) == 0 ? 1 : -1];
typedef char EntryContainsNativePointer[
    sizeof(struct ContentConsumerEntry) == sizeof(void *) ? 1 : -1];
typedef char UnitFlagAtExpectedOffset[
    offsetof(struct UnitFieldView, half18) + 8 * sizeof(u16) == 0x28 ? 1 : -1];
typedef char UnitFlagExtentIsSufficient[
    offsetof(struct UnitFieldView, half18) + 9 * sizeof(u16) >= 0x2A ? 1 : -1];

static void initialize_fixture(struct Fixture *fixture)
{
    memset(fixture, 0xA5, sizeof(*fixture));
    fixture->entries[0].object = &fixture->units[0];
    fixture->entries[1].object = &fixture->units[1];
    fixture->context.selected = &fixture->entries[1];
    fixture->context.guard = 0xC17E5A9Bu;
}

static void check_real_field_helpers(void)
{
    static const u16 flags[] = {0x0000, 0x1000, 0xA800, 0x5008};
    static const u32 expected_field[] = {0x0000, 0x1000, 0xA800, 0x5008};
    static const u8 expected_bit12[] = {0, 1, 0, 1};
    struct UnitFieldView units[4];
    struct UnitFieldView before[4];
    unsigned i;

    memset(units, 0x39, sizeof(units));
    for (i = 0; i < 4; i++)
        units[i].half18[8] = flags[i];
    memcpy(before, units, sizeof(units));
    for (i = 0; i < 4; i++) {
        assert(ReadUnitField(&units[i], 27) == expected_field[i]);
        assert(UnitField27HasBit12(&units[i]) == expected_bit12[i]);
    }
    assert(UnitField27HasBit12(0) == 0);
    assert(memcmp(units, before, sizeof(units)) == 0);
}

static void check_match_stores_entry(void)
{
    struct Fixture fixture;
    struct Fixture expected;
    ContentEntryCallback callback = ContentUnitSelectCallback;
    int result;

    initialize_fixture(&fixture);
    fixture.units[0].half18[8] = 0x5008;
    expected = fixture;
    expected.context.selected = &fixture.entries[0];

    result = callback(&fixture.entries[0], &fixture.context.selected);
    assert(result == 0);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
}

static void check_nonmatch_preserves_output(void)
{
    struct Fixture fixture;
    struct Fixture expected;
    ContentEntryCallback callback = ContentUnitSelectCallback;
    int result;

    initialize_fixture(&fixture);
    fixture.units[0].half18[8] = 0xA800;
    expected = fixture;

    result = callback(&fixture.entries[0], &fixture.context.selected);
    assert(result == 1);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
}

static void check_null_unit_is_a_nonmatch(void)
{
    struct Fixture fixture;
    struct Fixture expected;
    ContentEntryCallback callback = ContentUnitSelectCallback;
    int result;

    initialize_fixture(&fixture);
    fixture.entries[0].object = 0;
    expected = fixture;

    result = callback(&fixture.entries[0], &fixture.context.selected);
    assert(result == 1);
    assert(memcmp(&fixture, &expected, sizeof(fixture)) == 0);
}

int main(void)
{
    assert(sizeof(void *) == 4 || sizeof(void *) == 8);
    check_real_field_helpers();
    check_match_stores_entry();
    check_nonmatch_preserves_output();
    check_null_unit_is_a_nonmatch();
    return 0;
}
