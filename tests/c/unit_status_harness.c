/* Portable behavior of the real recovered state routines. */
#include <assert.h>
#include <string.h>
#include "../../src/unit_status.c"
#include "../../src/unit_state_access.c"
#include "../../src/unit_counters.c"

u8 UnitField19IsZero(const struct UnitFieldView *unit)
{
    const u8 *bytes = (const u8 *)unit;
    return bytes[0x18] == 0 && bytes[0x19] == 0;
}
static void test_counters(void)
{
    u8 unit[0x108], expected[0x108];
    unsigned first, second, value, mode;
    for (first = 0; first < 256; first++) {
        for (second = 0; second < 256; second++) {
            memset(unit, 0xa5, sizeof(unit));
            memcpy(expected, unit, sizeof(unit));
            expected[0xda] = first > second ? first - second : 0;
            expected[0xdb] = second > first ? second - first : 0;
            expected[0xea] &= ~0x60;
            if (first > second) expected[0xea] |= 0x20;
            if (second > first) expected[0xea] |= 0x40;
            SetOpposingStateCounters(unit, first, second);
            assert(memcmp(unit, expected, sizeof(unit)) == 0);
        }
    }
    for (value = 0; value < 256; value++) {
        for (mode = 0; mode < 4; mode++) {
            unsigned offset = mode & 1 ? 0xda : 0xdb;
            unsigned mask = mode & 1 ? 0x20 : 0x40;
            memset(unit, 0xa5, sizeof(unit));
            unit[0xea] = 0x9f | (mode << 5);
            unit[0xda] = unit[0xdb] = value;
            memcpy(expected, unit, sizeof(unit));
            if (mode && value < 128) {
                expected[offset] = value ? value - 1 : 0;
                if (value <= 1) expected[0xea] &= ~mask;
            }
            TickOpposingStateCounters(unit);
            assert(memcmp(unit, expected, sizeof(unit)) == 0);
        }
    }
}
static void test_exclusions(void)
{
    u8 unit[0x108], expected[0x108];
    unsigned value, state, blocked;
    static const u16 excluded[] = {0, 172, 221, 65535};
    for (blocked = 0; blocked < 5; blocked++) {
        for (state = 0; state < 256; state++) {
            for (value = 0; value < 4; value++) {
                unsigned reject = blocked || value == 1 || value == 2;
                memset(unit, 0, sizeof(unit));
                unit[0x18] = 1;
                if (blocked == 1) unit[0x18] = 0;
                if (blocked == 2) unit[0xe8] = 0x40;
                if (blocked == 3) unit[0xea] = 0x80;
                if (blocked == 4) unit[0xeb] = 4;
                unit[0xe9] = state;
                memcpy(expected, unit, sizeof(unit));
                assert(UnitHasBlockingState(unit) == (blocked != 0));
                if (!reject) expected[0xe9] &= ~0x50;
                assert(ClearUnitStateUnlessExcluded(unit, excluded[value]) == !reject);
                assert(memcmp(unit, expected, sizeof(unit)) == 0);
            }
        }
    }
}
static void test_indexed_bits(void)
{
    u8 unit[0x108], expected[0x108];
    unsigned index, value;
    for (index = 0; index < 48; index++) {
        for (value = 0; value < 256; value++) {
            unsigned mask = 1u << (index % 8);
            memset(unit, 0xa5, sizeof(unit));
            memcpy(expected, unit, sizeof(unit));
            expected[0xe8 + index / 8] &= ~mask;
            if (value) expected[0xe8 + index / 8] |= mask;
            SetIndexedUnitState(unit, index, value);
            assert(memcmp(unit, expected, sizeof(unit)) == 0);
        }
    }
    memset(unit, 0, sizeof(unit));
    for (value = 0; value < 256; value++) {
        unit[0xe8] = value;
        assert(UnitStateE8Bit1(unit) == ((value >> 1) & 1));
        assert(UnitStateE8Bit6(unit) == ((value >> 6) & 1));
        unit[0xec] = value;
        assert(UnitStateECBit0(unit) == (value & 1));
        assert(UnitStateECBit1(unit) == ((value >> 1) & 1));
        assert(UnitStateECBit2(unit) == ((value >> 2) & 1));
    }
}
static void test_fixed_accessors(void)
{
    static const struct {
        unsigned offset, bit;
        void (*set)(u8 *, u8);
        u8 (*get)(const u8 *);
    } cases[] = {
        {0xe8, 1, SetUnitStateE8Bit1, UnitStateE8Bit1},
        {0xe8, 2, SetUnitStateE8Bit2, UnitStateE8Bit2},
        {0xe8, 3, SetUnitStateE8Bit3, UnitStateE8Bit3},
        {0xe8, 4, SetUnitStateE8Bit4, UnitStateE8Bit4},
        {0xe8, 5, SetUnitStateE8Bit5, UnitStateE8Bit5},
        {0xe8, 6, SetUnitStateE8Bit6, UnitStateE8Bit6},
        {0xe8, 7, SetUnitStateE8Bit7, UnitStateE8Bit7},
        {0xe9, 0, SetUnitStateE9Bit0, UnitStateE9Bit0},
        {0xe9, 1, SetUnitStateE9Bit1, UnitStateE9Bit1},
        {0xe9, 2, SetUnitStateE9Bit2, UnitStateE9Bit2},
        {0xe9, 3, SetUnitStateE9Bit3, UnitStateE9Bit3},
        {0xe9, 4, SetUnitStateE9Bit4, UnitStateE9Bit4},
        {0xe9, 5, SetUnitStateE9Bit5, UnitStateE9Bit5},
        {0xe9, 6, SetUnitStateE9Bit6, UnitStateE9Bit6},
        {0xe9, 7, SetUnitStateE9Bit7, UnitStateE9Bit7},
        {0xea, 0, SetUnitStateEABit0, UnitStateEABit0},
        {0xea, 1, SetUnitStateEABit1, UnitStateEABit1},
        {0xea, 2, SetUnitStateEABit2, UnitStateEABit2},
        {0xea, 3, SetUnitStateEABit3, UnitStateEABit3},
        {0xea, 4, SetUnitStateEABit4, UnitStateEABit4},
        {0xea, 5, SetUnitStateEABit5, UnitStateEABit5},
        {0xea, 6, SetUnitStateEABit6, UnitStateEABit6},
        {0xea, 7, SetUnitStateEABit7, UnitStateEABit7},
        {0xeb, 0, SetUnitStateEBBit0, UnitStateEBBit0},
        {0xeb, 1, SetUnitStateEBBit1, UnitStateEBBit1},
        {0xeb, 2, SetUnitStateEBBit2, UnitStateEBBit2},
        {0xeb, 3, SetUnitStateEBBit3, UnitStateEBBit3},
        {0xeb, 4, SetUnitStateEBBit4, UnitStateEBBit4},
        {0xeb, 5, SetUnitStateEBBit5, UnitStateEBBit5},
        {0xeb, 6, SetUnitStateEBBit6, UnitStateEBBit6},
        {0xeb, 7, SetUnitStateEBBit7, UnitStateEBBit7},
        {0xec, 0, SetUnitStateECBit0, UnitStateECBit0},
        {0xec, 1, SetUnitStateECBit1, UnitStateECBit1},
        {0xec, 2, SetUnitStateECBit2, UnitStateECBit2},
        {0xec, 3, SetUnitStateECBit3, UnitStateECBit3},
        {0xec, 4, SetUnitStateECBit4, UnitStateECBit4},
        {0xec, 5, SetUnitStateECBit5, UnitStateECBit5},
        {0xec, 6, SetUnitStateECBit6, UnitStateECBit6},
        {0xec, 7, SetUnitStateECBit7, UnitStateECBit7},
        {0xed, 0, SetUnitStateEDBit0, UnitStateEDBit0},
        {0xed, 1, SetUnitStateEDBit1, UnitStateEDBit1},
        {0xed, 2, SetUnitStateEDBit2, UnitStateEDBit2},
        {0xed, 3, SetUnitStateEDBit3, UnitStateEDBit3},
        {0xed, 4, SetUnitStateEDBit4, UnitStateEDBit4},
        {0xed, 5, SetUnitStateEDBit5, UnitStateEDBit5},
        {0xed, 6, SetUnitStateEDBit6, UnitStateEDBit6},
        {0xed, 7, SetUnitStateEDBit7, UnitStateEDBit7},
    };
    static const u8 values[] = {0, 1, 2, 128, 255};
    u8 unit[0x108], expected[0x108];
    unsigned i, before, v;
    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        unsigned offset = cases[i].offset, mask = 1u << cases[i].bit;
        for (before = 0; before < 256; before++) {
            for (v = 0; v < sizeof(values); v++) {
                memset(unit, 0xa5, sizeof(unit));
                unit[offset] = before;
                assert(cases[i].get(unit) == ((before & mask) != 0));
                memcpy(expected, unit, sizeof(unit));
                expected[offset] = (before & ~mask) | (values[v] ? mask : 0);
                cases[i].set(unit, values[v]);
                assert(memcmp(unit, expected, sizeof(unit)) == 0);
                assert(cases[i].get(unit) == (values[v] != 0));
            }
        }
    }
}
static void test_counter_accessors(void)
{
    static const struct {
        unsigned offset;
        u8 (*get)(const u8 *);
        void (*set)(u8 *, u8);
    } cases[] = {
        {0xd8, ReadUnitCounterD8, WriteUnitCounterD8},
        {0xd9, ReadUnitCounterD9, WriteUnitCounterD9},
        {0xda, ReadUnitCounterDA, WriteUnitCounterDA},
        {0xdb, ReadUnitCounterDB, WriteUnitCounterDB},
        {0xdc, ReadUnitCounterDC, WriteUnitCounterDC},
        {0xdd, ReadUnitCounterDD, WriteUnitCounterDD},
        {0xde, ReadUnitCounterDE, WriteUnitCounterDE},
        {0xdf, ReadUnitCounterDF, WriteUnitCounterDF},
        {0xe0, ReadUnitCounterE0, WriteUnitCounterE0},
        {0xe1, ReadUnitCounterE1, WriteUnitCounterE1},
        {0xe2, ReadUnitCounterE2, WriteUnitCounterE2},
        {0xe3, ReadUnitCounterE3, WriteUnitCounterE3},
        {0xe4, ReadUnitCounterE4, WriteUnitCounterE4},
        {0xe5, ReadUnitCounterE5, WriteUnitCounterE5},
        {0xe6, ReadUnitCounterE6, WriteUnitCounterE6},
    };
    u8 unit[0x108], expected[0x108];
    unsigned i, value;
    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        for (value = 0; value < 256; value++) {
            memset(unit, 0xa5, sizeof(unit));
            memcpy(expected, unit, sizeof(unit));
            expected[cases[i].offset] = value;
            cases[i].set(unit, value);
            assert(cases[i].get(unit) == value);
            assert(memcmp(unit, expected, sizeof(unit)) == 0);
        }
    }
}
static void test_counter_add(void)
{
    static const u8 sides[] = {0, 1, 2, 128, 255};
    u8 unit[0x108], expected[0x108];
    unsigned first, second, side;
    for (first = 0; first < 256; first++) {
        for (second = 0; second < 256; second++) {
            for (side = 0; side < sizeof(sides); side++) {
                unsigned a = first, b = second;
                memset(unit, 0xa5, sizeof(unit));
                unit[0xda] = first;
                unit[0xdb] = second;
                memcpy(expected, unit, sizeof(unit));
                if (first < 128 && second < 128) {
                    if (sides[side]) { a += 3; if (a > 6) a = 6; }
                    else { b += 3; if (b > 6) b = 6; }
                    expected[0xda] = a > b ? a - b : 0;
                    expected[0xdb] = b > a ? b - a : 0;
                    expected[0xea] &= ~0x60;
                    if (a > b) expected[0xea] |= 0x20;
                    if (b > a) expected[0xea] |= 0x40;
                }
                AddOpposingStateCounter(unit, sides[side]);
                assert(memcmp(unit, expected, sizeof(unit)) == 0);
            }
        }
    }
}
int main(int argc, char **argv)
{
    assert(argc == 2);
    if (!strcmp(argv[1], "counters")) test_counters();
    else if (!strcmp(argv[1], "exclusions")) test_exclusions();
    else if (!strcmp(argv[1], "bits")) test_indexed_bits();
    else if (!strcmp(argv[1], "fixed")) test_fixed_accessors();
    else if (!strcmp(argv[1], "counter-accessors")) test_counter_accessors();
    else if (!strcmp(argv[1], "counter-add")) test_counter_add();
    else assert(0);
    return 0;
}
