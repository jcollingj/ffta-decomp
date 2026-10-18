#include <assert.h>
#include <string.h>
#include "unit_equipped_stats.h"

struct Call { u16 item; u8 selector; };
static struct UnitFieldView sUnit;
static unsigned sExpectedSelector;
static u32 sValues[65536];
static struct Call sCalls[8];
static unsigned sCallCount;
static int sMutate;

u32 GetItemProperty(u16 item, u8 selector)
{
    assert(item != 0);
    assert(selector == sExpectedSelector);
    assert(sCallCount < sizeof(sCalls) / sizeof(sCalls[0]));
    sCalls[sCallCount].item = item;
    sCalls[sCallCount].selector = selector;
    sCallCount++;
    if (sMutate && sCallCount == 1) {
        sUnit.half18[4] = 777;
        sUnit.half18[13] = 4;
    }
    return sValues[item];
}

static void reset_calls(u8 selector)
{
    sExpectedSelector = selector;
    sCallCount = 0;
    sMutate = 0;
    memset(sCalls, 0, sizeof(sCalls));
    memset(sValues, 0, sizeof(sValues));
}

static void assert_calls(const u16 *items, unsigned count, u8 selector)
{
    unsigned i;
    assert(sCallCount == count);
    for (i = 0; i < count; i++) {
        assert(sCalls[i].item == items[i]);
        assert(sCalls[i].selector == selector);
    }
}

static void test_each_offset_selector_and_no_call_path(void)
{
    struct UnitFieldView before, expected;
    static const u16 none[1] = {0};
    memset(&sUnit, 0x5a, sizeof(sUnit));
    sUnit.half18[4] = 112;
    sUnit.half18[5] = 223;
    sUnit.half18[6] = 334;
    sUnit.half18[7] = 445;
    sUnit.half18[9] = 1;
    sUnit.half18[10] = 0;
    sUnit.half18[11] = 2;
    sUnit.half18[12] = 0;
    sUnit.half18[13] = 3;
    memcpy(&before, &sUnit, sizeof(before));
    memcpy(&expected, &before, sizeof(expected));

    reset_calls(10);
    assert(GetUnitEquippedStat20(&sUnit, 0) == 112);
    assert_calls(none, 0, 10);
    reset_calls(12);
    assert(GetUnitEquippedStat24(&sUnit, (u8)0x100) == 334);
    assert_calls(none, 0, 12);
    reset_calls(11);
    assert(GetUnitEquippedStat22(&sUnit, (u8)0x100) == 223);
    assert_calls(none, 0, 11);
    reset_calls(13);
    assert(GetUnitEquippedStat26(&sUnit, 0) == 445);
    assert_calls(none, 0, 13);
    assert(memcmp(&sUnit, &expected, sizeof(expected)) == 0);
}

static void test_truthy_low_byte_all_five_slots_and_reload(void)
{
    static const u16 expected_items[] = {1, 1, 2, 4};
    struct UnitFieldView expected;
    memset(&sUnit, 0x39, sizeof(sUnit));
    sUnit.half18[4] = 50;
    sUnit.half18[9] = 1;
    sUnit.half18[10] = 0;
    sUnit.half18[11] = 1;
    sUnit.half18[12] = 2;
    sUnit.half18[13] = 3;
    sValues[1] = 100;
    sValues[2] = 200;
    sValues[3] = 300;
    sValues[4] = 5;
    reset_calls(10);
    sValues[1] = 100;
    sValues[2] = 200;
    sValues[4] = 5;
    sMutate = 1;
    memcpy(&expected, &sUnit, sizeof(expected));
    expected.half18[4] = 777;
    expected.half18[13] = 4;
    assert(GetUnitEquippedStat20(&sUnit, (u8)0x101) == 455);
    assert_calls(expected_items, 4, 10);
    assert(memcmp(&sUnit, &expected, sizeof(expected)) == 0);
}

static void test_signed_wrap_upper_cap_and_negative_results(void)
{
    struct UnitFieldView before, expected;
    static const u16 one[] = {5};
    static const u16 two[] = {5, 6};

    memset(&sUnit, 0x83, sizeof(sUnit));
    sUnit.half18[6] = 0x7fff;
    sUnit.half18[9] = 5;
    sUnit.half18[10] = sUnit.half18[11] = sUnit.half18[12] = sUnit.half18[13] = 0;
    memcpy(&before, &sUnit, sizeof(before));
    memcpy(&expected, &before, sizeof(expected));
    reset_calls(12);
    sValues[5] = 1;
    assert(GetUnitEquippedStat24(&sUnit, 1) == -32768);
    assert_calls(one, 1, 12);
    assert(memcmp(&sUnit, &expected, sizeof(expected)) == 0);

    sUnit.half18[6] = 900;
    memcpy(&before, &sUnit, sizeof(before));
    memcpy(&expected, &before, sizeof(expected));
    reset_calls(12);
    sValues[5] = 200;
    assert(GetUnitEquippedStat24(&sUnit, 1) == 999);
    assert_calls(one, 1, 12);
    assert(memcmp(&sUnit, &expected, sizeof(expected)) == 0);

    sUnit.half18[5] = (u16)-5;
    memcpy(&before, &sUnit, sizeof(before));
    memcpy(&expected, &before, sizeof(expected));
    reset_calls(11);
    assert(GetUnitEquippedStat22(&sUnit, 0) == -5);
    assert_calls(one, 0, 11);
    assert(memcmp(&sUnit, &expected, sizeof(expected)) == 0);

    sUnit.half18[5] = 0;
    memcpy(&before, &sUnit, sizeof(before));
    memcpy(&expected, &before, sizeof(expected));
    reset_calls(11);
    sValues[5] = 0x8000;
    assert(GetUnitEquippedStat22(&sUnit, 1) == -32768);
    assert_calls(one, 1, 11);
    assert(memcmp(&sUnit, &expected, sizeof(expected)) == 0);

    sUnit.half18[7] = 10;
    sUnit.half18[10] = 6;
    memcpy(&before, &sUnit, sizeof(before));
    memcpy(&expected, &before, sizeof(expected));
    reset_calls(13);
    sValues[5] = 0xffffffffu;
    sValues[6] = 2;
    assert(GetUnitEquippedStat26(&sUnit, 1) == 11);
    assert_calls(two, 2, 13);
    assert(memcmp(&sUnit, &expected, sizeof(expected)) == 0);

    sUnit.half18[9] = sUnit.half18[10] = sUnit.half18[11] =
        sUnit.half18[12] = sUnit.half18[13] = 0;
    sUnit.half18[4] = 31;
    memcpy(&before, &sUnit, sizeof(before));
    memcpy(&expected, &before, sizeof(expected));
    reset_calls(10);
    assert(GetUnitEquippedStat20(&sUnit, 1) == 31);
    assert_calls(one, 0, 10);
    assert(memcmp(&sUnit, &expected, sizeof(expected)) == 0);

    sUnit.half18[4] = 1500;
    memcpy(&before, &sUnit, sizeof(before));
    memcpy(&expected, &before, sizeof(expected));
    reset_calls(10);
    assert(GetUnitEquippedStat20(&sUnit, 0) == 999);
    assert_calls(one, 0, 10);
    assert(memcmp(&sUnit, &expected, sizeof(expected)) == 0);
}

static void test_repeated_ids_without_mutation(void)
{
    static const u16 items[] = {7, 7, 8};
    struct UnitFieldView before, expected;
    memset(&sUnit, 0x24, sizeof(sUnit));
    sUnit.half18[5] = 20;
    sUnit.half18[9] = 7;
    sUnit.half18[10] = 7;
    sUnit.half18[11] = 0;
    sUnit.half18[12] = 8;
    sUnit.half18[13] = 0;
    memcpy(&before, &sUnit, sizeof(before));
    memcpy(&expected, &before, sizeof(expected));
    reset_calls(11);
    sValues[7] = 3;
    sValues[8] = 4;
    assert(GetUnitEquippedStat22(&sUnit, 255) == 30);
    assert_calls(items, 3, 11);
    assert(memcmp(&sUnit, &expected, sizeof(expected)) == 0);
}

int main(void)
{
    test_each_offset_selector_and_no_call_path();
    test_truthy_low_byte_all_five_slots_and_reload();
    test_signed_wrap_upper_cap_and_negative_results();
    test_repeated_ids_without_mutation();
    return 0;
}
