/* Observed state bits; full gameplay meanings remain under investigation. */
#include "unit.h"
#include "unit_status.h"
static inline u8 TestState(const u8 *flags, u8 mask)
{
    if (*flags & mask) return 1;
    return 0;
}
static inline void ClearState(u8 *flags, u8 mask)
{ *flags &= ~mask; }
u8 UnitHasBlockingState(const u8 *unit)
{
    if (UnitField19IsZero((const struct UnitFieldView *)unit)
        || TestState(unit + 0xe8, 0x40)
        || TestState(unit + 0xea, 0x80)
        || TestState(unit + 0xeb, 4)) return 1;
    return 0;
}
u8 ClearUnitStateUnlessExcluded(u8 *unit, u16 value)
{
    if (value == 221 || value == 172) return 0;
    if (UnitHasBlockingState(unit)) return 0;
    if (TestState(unit + 0xe9, 0x10)) ClearState(unit + 0xe9, 0x10);
    if (TestState(unit + 0xe9, 0x40)) ClearState(unit + 0xe9, 0x40);
    return 1;
}

static inline void SetState(u8 *flags, u8 mask)
{ *flags = (*flags & ~mask) | mask; }
void SetOpposingStateCounters(u8 *unit, u8 first, u8 second)
{
    if (first >= second) { first -= second; second = 0; }
    else { second -= first; first = 0; }
    unit[0xda] = first;
    if (!first) ClearState(unit + 0xea, 0x20);
    else SetState(unit + 0xea, 0x20);
    unit[0xdb] = second;
    if (!second) ClearState(unit + 0xea, 0x40);
    else SetState(unit + 0xea, 0x40);
}
void TickOpposingStateCounters(u8 *unit)
{
    if (TestState(unit + 0xea, 0x20)) {
        u8 counter = unit[0xda];
        if (counter & 0x80) return;
        if (counter) counter--;
        if (!counter) ClearState(unit + 0xea, 0x20);
        unit[0xda] = counter;
    } else if (TestState(unit + 0xea, 0x40)) {
        u8 counter = unit[0xdb];
        if (counter & 0x80) return;
        if (counter) counter--;
        if (!counter) ClearState(unit + 0xea, 0x40);
        unit[0xdb] = counter;
    }
}

void SetIndexedUnitState(u8 *unit, u16 index, u8 value)
{
    u8 *flags = &unit[0xe8 + index / 8];
    u8 mask = 1 << (index % 8);
    *flags &= ~mask;
    if (value) *flags |= mask;
}
u8 UnitStateE8Bit1(const u8 *unit) { return TestState(unit + 0xe8, 2); }
u8 UnitStateE8Bit2(const u8 *unit) { return TestState(unit + 0xe8, 4); }
u8 UnitStateE8Bit3(const u8 *unit) { return TestState(unit + 0xe8, 8); }
u8 UnitStateE8Bit4(const u8 *unit) { return TestState(unit + 0xe8, 16); }
u8 UnitStateE8Bit5(const u8 *unit) { return TestState(unit + 0xe8, 32); }
u8 UnitStateE8Bit6(const u8 *unit) { return TestState(unit + 0xe8, 64); }
u8 UnitStateE8Bit7(const u8 *unit) { return TestState(unit + 0xe8, 128); }
static inline u8 TestStateAt(const u8 *unit, unsigned offset, u8 mask)
{
    if (unit[offset] & mask) return 1;
    return 0;
}
u8 UnitStateE9Bit0(const u8 *unit) { return TestStateAt(unit, 0xe9, 1); }
u8 UnitStateE9Bit1(const u8 *unit) { return TestState(unit + 0xe9, 2); }
u8 UnitStateE9Bit2(const u8 *unit) { return TestState(unit + 0xe9, 4); }
u8 UnitStateE9Bit3(const u8 *unit) { return TestState(unit + 0xe9, 8); }
u8 UnitStateE9Bit4(const u8 *unit) { return TestState(unit + 0xe9, 16); }
u8 UnitStateE9Bit5(const u8 *unit) { return TestState(unit + 0xe9, 32); }
u8 UnitStateE9Bit6(const u8 *unit) { return TestState(unit + 0xe9, 64); }
u8 UnitStateE9Bit7(const u8 *unit) { return TestState(unit + 0xe9, 128); }
u8 UnitStateEABit0(const u8 *unit) { return TestState(unit + 0xea, 1); }
u8 UnitStateECBit2(const u8 *unit) { return TestStateAt(unit, 0xec, 4); }
u8 UnitStateECBit1(const u8 *unit) { return TestState(unit + 0xec, 2); }
u8 UnitStateEABit1(const u8 *unit) { return TestStateAt(unit, 0xea, 2); }
u8 UnitStateEABit2(const u8 *unit) { return TestState(unit + 0xea, 4); }
u8 UnitStateEABit4(const u8 *unit) { return TestState(unit + 0xea, 16); }
u8 UnitStateEABit5(const u8 *unit) { return TestState(unit + 0xea, 32); }
u8 UnitStateEABit6(const u8 *unit) { return TestState(unit + 0xea, 64); }
u8 UnitStateEABit7(const u8 *unit) { return TestState(unit + 0xea, 128); }
u8 UnitStateEBBit0(const u8 *unit) { return TestState(unit + 0xeb, 1); }
u8 UnitStateEBBit1(const u8 *unit) { return TestState(unit + 0xeb, 2); }
u8 UnitStateEBBit2(const u8 *unit) { return TestState(unit + 0xeb, 4); }
u8 UnitStateEBBit3(const u8 *unit) { return TestState(unit + 0xeb, 8); }
u8 UnitStateEBBit4(const u8 *unit) { return TestState(unit + 0xeb, 16); }
u8 UnitStateEBBit5(const u8 *unit) { return TestState(unit + 0xeb, 32); }
u8 UnitStateEBBit6(const u8 *unit) { return TestState(unit + 0xeb, 64); }
u8 UnitStateEBBit7(const u8 *unit) { return TestState(unit + 0xeb, 128); }
u8 UnitStateECBit0(const u8 *unit) { return TestState(unit + 0xec, 1); }
