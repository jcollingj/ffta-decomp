#include <assert.h>
#include <string.h>
#include "unit_stat_wrappers.h"

enum { STAT20, STAT24, STAT22, STAT26, STAT_COUNT };
struct UnitFixture {
    u8 before[16];
    struct UnitFieldView unit;
    u8 after[16];
};
struct Call {
    unsigned stat;
    const struct UnitFieldView *unit;
    u8 flag;
};

static struct Call sCalls[4];
static unsigned sCallCount;
static signed int sReturns[STAT_COUNT];

static signed int record_call(unsigned stat,
                              const struct UnitFieldView *unit, u8 flag)
{
    assert(sCallCount < sizeof(sCalls) / sizeof(sCalls[0]));
    sCalls[sCallCount].stat = stat;
    sCalls[sCallCount].unit = unit;
    sCalls[sCallCount].flag = flag;
    sCallCount++;
    return sReturns[stat];
}

signed int GetUnitEquippedStat20(const struct UnitFieldView *unit, u8 flag)
{
    return record_call(STAT20, unit, flag);
}

signed int GetUnitEquippedStat24(const struct UnitFieldView *unit, u8 flag)
{
    return record_call(STAT24, unit, flag);
}

signed int GetUnitEquippedStat22(const struct UnitFieldView *unit, u8 flag)
{
    return record_call(STAT22, unit, flag);
}

signed int GetUnitEquippedStat26(const struct UnitFieldView *unit, u8 flag)
{
    return record_call(STAT26, unit, flag);
}

static void reset_calls(void)
{
    sCallCount = 0;
    memset(sCalls, 0, sizeof(sCalls));
}

static void assert_trace(unsigned expected_stat, unsigned expected_count,
                         const struct UnitFieldView *unit, u8 flag)
{
    assert(sCallCount == expected_count);
    if (expected_count) {
        assert(sCalls[0].stat == expected_stat);
        assert(sCalls[0].unit == unit);
        assert(sCalls[0].flag == flag);
    }
}

static signed int expected_cap(unsigned index)
{
    static const signed int values[] = { -32768, 998, 999, 999, 999 };
    assert(index < sizeof(values) / sizeof(values[0]));
    return values[index];
}

static void test_dispatchers(void)
{
    static const unsigned char selected20or24[] = {
        STAT_COUNT, STAT20, STAT24, STAT20
    };
    static const unsigned char selected22or26[] = {
        STAT_COUNT, STAT22, STAT26, STAT22
    };
    struct UnitFixture fixture, before;
    unsigned mask, flag;
    unsigned raw_mask, raw_flag;
    memset(&fixture, 0x6d, sizeof(fixture));
    memcpy(&before, &fixture, sizeof(before));
    sReturns[STAT20] = -321;
    sReturns[STAT24] = 1000;
    sReturns[STAT22] = 32767;
    sReturns[STAT26] = -32768;

    for (mask = 0; mask < 256; mask++) {
        unsigned index = mask & 3;
        unsigned expected;
        for (flag = 0; flag < 256; flag++) {
            expected = selected20or24[index];
            reset_calls();
            if (expected == STAT_COUNT) {
                assert(SelectUnitEquippedStat20Or24(&fixture.unit,
                       (u8)mask, (u8)flag) == 0);
                assert_trace(0, 0, &fixture.unit, (u8)flag);
            } else {
                signed int result = expected == STAT20 ? -321 : 1000;
                assert(SelectUnitEquippedStat20Or24(&fixture.unit,
                       (u8)mask, (u8)flag) == result);
                assert_trace(expected, 1, &fixture.unit, (u8)flag);
            }

            expected = selected22or26[index];
            reset_calls();
            if (expected == STAT_COUNT) {
                assert(SelectUnitEquippedStat22Or26(&fixture.unit,
                       (u8)mask, (u8)flag) == 0);
                assert_trace(0, 0, &fixture.unit, (u8)flag);
            } else {
                signed int result = expected == STAT22 ? 32767 : -32768;
                assert(SelectUnitEquippedStat22Or26(&fixture.unit,
                       (u8)mask, (u8)flag) == result);
                assert_trace(expected, 1, &fixture.unit, (u8)flag);
            }
        }
    }

    reset_calls();
    raw_mask = 0x101;
    raw_flag = 0x101;
    assert(SelectUnitEquippedStat20Or24(&fixture.unit, raw_mask, raw_flag) == -321);
    assert_trace(STAT20, 1, &fixture.unit, 1);
    reset_calls();
    raw_mask = 0x102;
    raw_flag = 0x100;
    assert(SelectUnitEquippedStat22Or26(&fixture.unit, raw_mask, raw_flag) == -32768);
    assert_trace(STAT26, 1, &fixture.unit, 0);
    assert(memcmp(&fixture, &before, sizeof(fixture)) == 0);
}

static void test_capped_dispatchers(void)
{
    static const unsigned char selected20or24[] = {
        STAT_COUNT, STAT20, STAT24, STAT20
    };
    static const unsigned char selected22or26[] = {
        STAT_COUNT, STAT22, STAT26, STAT22
    };
    struct UnitFixture fixture, before;
    unsigned mask;
    memset(&fixture, 0x92, sizeof(fixture));
    memcpy(&before, &fixture, sizeof(before));
    sReturns[STAT20] = 1000;
    sReturns[STAT24] = -32768;
    sReturns[STAT22] = 999;
    sReturns[STAT26] = 32767;

    for (mask = 0; mask < 256; mask++) {
        unsigned expected = selected20or24[mask & 3];
        signed int result;
        reset_calls();
        result = SelectUnitEquippedStat20Or24Capped(&fixture.unit, (u8)mask);
        if (expected == STAT_COUNT) {
            assert(result == 0);
            assert_trace(0, 0, &fixture.unit, 0);
        } else {
            signed int cap = expected == STAT20 ? 999 : -32768;
            assert(result == cap);
            assert_trace(expected, 1, &fixture.unit, 1);
        }

        expected = selected22or26[mask & 3];
        reset_calls();
        result = SelectUnitEquippedStat22Or26Capped(&fixture.unit, (u8)mask);
        if (expected == STAT_COUNT) {
            assert(result == 0);
            assert_trace(0, 0, &fixture.unit, 0);
        } else {
            assert(result == 999);
            assert_trace(expected, 1, &fixture.unit, 1);
        }
    }
    assert(memcmp(&fixture, &before, sizeof(fixture)) == 0);
}

static void test_single_stat_caps(void)
{
    static const signed int input[] = { -32768, 998, 999, 1000, 32767 };
    static const unsigned char expected_stat[] = { STAT20, STAT24, STAT22, STAT26 };
    struct UnitFixture fixture, before;
    unsigned i, stat;
    signed int (*functions[])(const struct UnitFieldView *) = {
        GetUnitStat20Capped, GetUnitStat24Capped,
        GetUnitStat22Capped, GetUnitStat26Capped
    };
    memset(&fixture, 0xa7, sizeof(fixture));
    memcpy(&before, &fixture, sizeof(before));

    for (stat = 0; stat < STAT_COUNT; stat++) {
        for (i = 0; i < sizeof(input) / sizeof(input[0]); i++) {
            reset_calls();
            sReturns[stat] = input[i];
            assert(functions[stat](&fixture.unit) == expected_cap(i));
            assert_trace(expected_stat[stat], 1, &fixture.unit, 1);
        }
    }
    assert(memcmp(&fixture, &before, sizeof(fixture)) == 0);
}

int main(void)
{
    test_dispatchers();
    test_capped_dispatchers();
    test_single_stat_caps();
    return 0;
}
