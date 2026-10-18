#include <assert.h>
#include <string.h>
#include "unit_equipment_sums.h"

#define UNIT_SIZE 0xFC
#define SLOT_COUNT 5
#define TRACE_CAPACITY 8

struct GuardedUnit {
    u8 before[8];
    struct UnitFieldView unit;
    u8 after[8];
};

struct ItemCall {
    u16 id;
    u8 property;
    u32 value;
};

static struct UnitFieldView *sLiveUnit;
static u32 sUnitValues[2];
static unsigned sUnitCalls;
static struct ItemCall sItemCalls[TRACE_CAPACITY];
static unsigned sItemCount;
static unsigned sItemIndex;
static unsigned sMutation;

u32 GetUnitProperty(const u8 *unit, u8 property)
{
    assert(unit == (const u8 *)sLiveUnit);
    assert(property == 30);
    assert(sUnitCalls < 2);
    return sUnitValues[sUnitCalls++];
}

u32 GetItemProperty(u16 id, u8 property)
{
    struct ItemCall *call;
    assert(sItemIndex < sItemCount);
    call = &sItemCalls[sItemIndex++];
    assert(call->id == id);
    assert(call->property == property);
    if (sMutation == 1 && sItemIndex == 1) {
        sLiveUnit->unknown34[9] = 1;
        sLiveUnit->half18[10] = 30;
    }
    return call->value;
}

static void reset_calls(struct UnitFieldView *unit, u32 first_unit_value,
                        u32 second_unit_value,
                        const struct ItemCall *items, unsigned item_count,
                        unsigned mutation)
{
    unsigned i;
    sLiveUnit = unit;
    sUnitValues[0] = first_unit_value;
    sUnitValues[1] = second_unit_value;
    sUnitCalls = 0;
    sItemCount = item_count;
    sItemIndex = 0;
    sMutation = mutation;
    for (i = 0; i < item_count; i++)
        sItemCalls[i] = items[i];
}

static void init_unit(struct GuardedUnit *guarded)
{
    memset(guarded, 0xA5, sizeof(*guarded));
    memset(&guarded->unit, 0, sizeof(guarded->unit));
    memset(guarded->before, 0x31, sizeof(guarded->before));
    memset(guarded->after, 0x72, sizeof(guarded->after));
}

static void expect_unit(const struct GuardedUnit *actual,
                        const struct GuardedUnit *expected)
{
    assert(memcmp(actual, expected, sizeof(*actual)) == 0);
}

static void expect_unit_calls(unsigned count)
{
    assert(sUnitCalls == count);
}

static void expect_item_calls(unsigned count)
{
    assert(sItemIndex == count);
}

int main(void)
{
    struct GuardedUnit actual, expected;
    static const struct ItemCall none[] = {{0, 0, 0}};
    static const struct ItemCall changed_items[] = {
        {10, 16, 2}, {30, 16, 3}
    };
    static const struct ItemCall sum_items[] = {
        {11, 255, 0xfffffff0u}, {12, 255, 0x30u}, {11, 255, 7}
    };
    static const struct ItemCall signed_items[] = {
        {21, 16, 0x80u}, {22, 16, 0x100u}
    };
    static const struct ItemCall regular_items[] = {{40, 16, 0}};
    static const struct ItemCall final_slot_items[] = {{99, 255, 0xa6u}};
    u32 result;

    assert(sizeof(struct UnitFieldView) == UNIT_SIZE);
    assert(__builtin_offsetof(struct UnitFieldView, half18) + 9 * sizeof(u16) == 0x2A);
    assert(__builtin_offsetof(struct UnitFieldView, half18) + 13 * sizeof(u16) == 0x32);
    assert(__builtin_offsetof(struct UnitFieldView, unknown34) + 9 == 0x3D);

    /* Empty equipment still calls the unit reader; scaled wrapper forwards zero. */
    init_unit(&actual);
    expected = actual;
    reset_calls(&actual.unit, 7, 200, none, 0, 0);
    assert(ReadUnitEquipmentValue16(&actual.unit) == 7);
    expect_unit_calls(1);
    expect_item_calls(0);
    expect_unit(&actual, &expected);

    reset_calls(&actual.unit, 7, 200, none, 0, 0);
    assert(ReadUnitEquipmentValue16Scaled10(&actual.unit) == 70);
    expect_unit_calls(1);
    expect_item_calls(0);
    expect_unit(&actual, &expected);

    /* Exact flag value 1 increments and wraps the narrowed base byte. */
    init_unit(&actual);
    actual.unit.unknown34[9] = 1;
    expected = actual;
    reset_calls(&actual.unit, 0x123400ffu, 0, none, 0, 0);
    assert(ReadUnitEquipmentValue16(&actual.unit) == 0);
    expect_unit_calls(1);
    expect_item_calls(0);
    expect_unit(&actual, &expected);

    init_unit(&actual);
    actual.unit.unknown34[9] = 1;
    expected = actual;
    reset_calls(&actual.unit, 6, 0, none, 0, 0);
    assert(ReadUnitEquipmentValue16(&actual.unit) == 7);
    expect_unit_calls(1);
    expect_item_calls(0);
    expect_unit(&actual, &expected);

    init_unit(&actual);
    actual.unit.unknown34[9] = 1;
    expected = actual;
    reset_calls(&actual.unit, 127, 0, none, 0, 0);
    assert(ReadUnitEquipmentValue16(&actual.unit) == 0);
    expect_unit_calls(1);
    expect_item_calls(0);
    expect_unit(&actual, &expected);

    /* Other truthy flag values do not increment; reader returns signed base. */
    init_unit(&actual);
    actual.unit.unknown34[9] = 2;
    expected = actual;
    reset_calls(&actual.unit, 126, 0, none, 0, 0);
    assert(ReadUnitEquipmentValue16(&actual.unit) == 126);
    expect_unit_calls(1);
    expect_item_calls(0);
    expect_unit(&actual, &expected);

    /* Base is captured once before item callbacks; subsequent slots are reloaded. */
    init_unit(&actual);
    actual.unit.half18[9] = 10;
    actual.unit.half18[10] = 20;
    expected = actual;
    expected.unit.unknown34[9] = 1;
    expected.unit.half18[10] = 30;
    reset_calls(&actual.unit, 5, 200, changed_items, 2, 1);
    result = ReadUnitEquipmentValue16(&actual.unit);
    assert(result == 10);
    expect_unit_calls(1);
    expect_item_calls(2);
    expect_unit(&actual, &expected);

    /* The sum is narrowed to signed byte before its negative clamp. */
    init_unit(&actual);
    actual.unit.half18[9] = 21;
    actual.unit.half18[10] = 22;
    expected = actual;
    reset_calls(&actual.unit, 0, 0, signed_items, 2, 0);
    assert(ReadUnitEquipmentValue16(&actual.unit) == 0);
    expect_unit_calls(1);
    expect_item_calls(2);
    expect_unit(&actual, &expected);

    /* Forwarder multiplies the returned byte value by ten. */
    init_unit(&actual);
    expected = actual;
    reset_calls(&actual.unit, 127, 0, none, 0, 0);
    assert(ReadUnitEquipmentValue16Scaled10(&actual.unit) == 1270);
    expect_unit_calls(1);
    expect_item_calls(0);
    expect_unit(&actual, &expected);

    /* Selector narrows to low byte; zero IDs skip; repeated IDs remain calls. */
    init_unit(&actual);
    actual.unit.half18[9] = 11;
    actual.unit.half18[10] = 0;
    actual.unit.half18[11] = 12;
    actual.unit.half18[12] = 11;
    expected = actual;
    reset_calls(&actual.unit, 0, 0, sum_items, 3, 0);
    assert(SumUnitEquipmentProperty(&actual.unit, 0x1234ffu) == 0x27u);
    expect_unit_calls(0);
    expect_item_calls(3);
    expect_unit(&actual, &expected);

    /* The final equipment halfword is visited; an empty set returns zero. */
    init_unit(&actual);
    expected = actual;
    reset_calls(&actual.unit, 0, 0, none, 0, 0);
    assert(SumUnitEquipmentProperty(&actual.unit, 0x1234ffu) == 0);
    expect_unit_calls(0);
    expect_item_calls(0);
    expect_unit(&actual, &expected);

    init_unit(&actual);
    actual.unit.half18[13] = 99;
    expected = actual;
    reset_calls(&actual.unit, 0, 0, final_slot_items, 1, 0);
    assert(SumUnitEquipmentProperty(&actual.unit, 0x1234ffu) == 0xa6u);
    expect_unit_calls(0);
    expect_item_calls(1);
    expect_unit(&actual, &expected);

    /* A later base read would differ; the helper must use the captured first value. */
    init_unit(&actual);
    actual.unit.half18[9] = 40;
    expected = actual;
    reset_calls(&actual.unit, 3, 250, regular_items, 1, 0);
    assert(ReadUnitEquipmentValue16(&actual.unit) == 3);
    expect_unit_calls(1);
    expect_item_calls(1);
    expect_unit(&actual, &expected);

    return 0;
}
