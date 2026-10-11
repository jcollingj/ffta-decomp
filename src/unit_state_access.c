/* Fixed unit state bit accessors; gameplay names remain unknown. */
#include "unit_status.h"
static inline u8 ReadFlag(const u8 *flags, u8 mask)
{ if (*flags & mask) return 1; return 0; }
static inline void WriteFlag(u8 *flags, u8 mask, u8 value)
{
    *flags &= ~mask;
    if (value) *flags |= mask;
}
u8 UnitStateEABit3(const u8 *unit) { return ReadFlag(unit + 0xea, 8); }
u8 UnitStateECBit3(const u8 *unit) { return ReadFlag(unit + 0xec, 8); }
u8 UnitStateECBit4(const u8 *unit) { return ReadFlag(unit + 0xec, 16); }
u8 UnitStateECBit5(const u8 *unit) { return ReadFlag(unit + 0xec, 32); }
u8 UnitStateECBit6(const u8 *unit) { return ReadFlag(unit + 0xec, 64); }
u8 UnitStateECBit7(const u8 *unit) { return ReadFlag(unit + 0xec, 128); }
u8 UnitStateEDBit0(const u8 *unit) { return ReadFlag(unit + 0xed, 1); }
u8 UnitStateEDBit1(const u8 *unit) { return ReadFlag(unit + 0xed, 2); }
u8 UnitStateEDBit2(const u8 *unit) { return ReadFlag(unit + 0xed, 4); }
u8 UnitStateEDBit3(const u8 *unit) { return ReadFlag(unit + 0xed, 8); }
u8 UnitStateEDBit4(const u8 *unit) { return ReadFlag(unit + 0xed, 16); }
u8 UnitStateEDBit5(const u8 *unit) { return ReadFlag(unit + 0xed, 32); }
u8 UnitStateEDBit6(const u8 *unit) { return ReadFlag(unit + 0xed, 64); }
u8 UnitStateEDBit7(const u8 *unit) { return ReadFlag(unit + 0xed, 128); }
void SetUnitStateE8Bit1(u8 *unit, u8 value) { WriteFlag(unit + 0xe8, 2, value); }
void SetUnitStateE8Bit2(u8 *unit, u8 value) { WriteFlag(unit + 0xe8, 4, value); }
void SetUnitStateE8Bit3(u8 *unit, u8 value) { WriteFlag(unit + 0xe8, 8, value); }
void SetUnitStateE8Bit4(u8 *unit, u8 value) { WriteFlag(unit + 0xe8, 16, value); }
void SetUnitStateE8Bit5(u8 *unit, u8 value) { WriteFlag(unit + 0xe8, 32, value); }
void SetUnitStateE8Bit6(u8 *unit, u8 value) { WriteFlag(unit + 0xe8, 64, value); }
void SetUnitStateE8Bit7(u8 *unit, u8 value) { WriteFlag(unit + 0xe8, 128, value); }
void SetUnitStateE9Bit0(u8 *unit, u8 value) { WriteFlag(unit + 0xe9, 1, value); }
void SetUnitStateE9Bit1(u8 *unit, u8 value) { WriteFlag(unit + 0xe9, 2, value); }
void SetUnitStateE9Bit2(u8 *unit, u8 value) { WriteFlag(unit + 0xe9, 4, value); }
void SetUnitStateE9Bit3(u8 *unit, u8 value) { WriteFlag(unit + 0xe9, 8, value); }
void SetUnitStateE9Bit4(u8 *unit, u8 value) { WriteFlag(unit + 0xe9, 16, value); }
void SetUnitStateE9Bit5(u8 *unit, u8 value) { WriteFlag(unit + 0xe9, 32, value); }
void SetUnitStateE9Bit6(u8 *unit, u8 value) { WriteFlag(unit + 0xe9, 64, value); }
void SetUnitStateE9Bit7(u8 *unit, u8 value) { WriteFlag(unit + 0xe9, 128, value); }
void SetUnitStateEABit0(u8 *unit, u8 value) { WriteFlag(unit + 0xea, 1, value); }
void SetUnitStateECBit2(u8 *unit, u8 value) { WriteFlag(unit + 0xec, 4, value); }
void SetUnitStateECBit1(u8 *unit, u8 value) { WriteFlag(unit + 0xec, 2, value); }
void SetUnitStateEABit1(u8 *unit, u8 value) { WriteFlag(unit + 0xea, 2, value); }
void SetUnitStateEABit2(u8 *unit, u8 value) { WriteFlag(unit + 0xea, 4, value); }
void SetUnitStateEABit4(u8 *unit, u8 value) { WriteFlag(unit + 0xea, 16, value); }
void SetUnitStateEABit5(u8 *unit, u8 value) { WriteFlag(unit + 0xea, 32, value); }
void SetUnitStateEABit6(u8 *unit, u8 value) { WriteFlag(unit + 0xea, 64, value); }
void SetUnitStateEABit7(u8 *unit, u8 value) { WriteFlag(unit + 0xea, 128, value); }
void SetUnitStateEBBit0(u8 *unit, u8 value) { WriteFlag(unit + 0xeb, 1, value); }
void SetUnitStateEBBit1(u8 *unit, u8 value) { WriteFlag(unit + 0xeb, 2, value); }
void SetUnitStateEBBit2(u8 *unit, u8 value) { WriteFlag(unit + 0xeb, 4, value); }
void SetUnitStateEBBit3(u8 *unit, u8 value) { WriteFlag(unit + 0xeb, 8, value); }
void SetUnitStateEBBit4(u8 *unit, u8 value) { WriteFlag(unit + 0xeb, 16, value); }
void SetUnitStateEBBit5(u8 *unit, u8 value) { WriteFlag(unit + 0xeb, 32, value); }
void SetUnitStateEBBit6(u8 *unit, u8 value) { WriteFlag(unit + 0xeb, 64, value); }
void SetUnitStateEBBit7(u8 *unit, u8 value) { WriteFlag(unit + 0xeb, 128, value); }
void SetUnitStateECBit0(u8 *unit, u8 value) { WriteFlag(unit + 0xec, 1, value); }
void SetUnitStateEABit3(u8 *unit, u8 value) { WriteFlag(unit + 0xea, 8, value); }
void SetUnitStateECBit3(u8 *unit, u8 value) { WriteFlag(unit + 0xec, 8, value); }
void SetUnitStateECBit4(u8 *unit, u8 value) { WriteFlag(unit + 0xec, 16, value); }
void SetUnitStateECBit5(u8 *unit, u8 value) { WriteFlag(unit + 0xec, 32, value); }
void SetUnitStateECBit6(u8 *unit, u8 value) { WriteFlag(unit + 0xec, 64, value); }
void SetUnitStateECBit7(u8 *unit, u8 value) { WriteFlag(unit + 0xec, 128, value); }
void SetUnitStateEDBit0(u8 *unit, u8 value) { WriteFlag(unit + 0xed, 1, value); }
void SetUnitStateEDBit1(u8 *unit, u8 value) { WriteFlag(unit + 0xed, 2, value); }
void SetUnitStateEDBit2(u8 *unit, u8 value) { WriteFlag(unit + 0xed, 4, value); }
void SetUnitStateEDBit3(u8 *unit, u8 value) { WriteFlag(unit + 0xed, 8, value); }
void SetUnitStateEDBit4(u8 *unit, u8 value) { WriteFlag(unit + 0xed, 16, value); }
void SetUnitStateEDBit5(u8 *unit, u8 value) { WriteFlag(unit + 0xed, 32, value); }
void SetUnitStateEDBit6(u8 *unit, u8 value) { WriteFlag(unit + 0xed, 64, value); }
void SetUnitStateEDBit7(u8 *unit, u8 value) { WriteFlag(unit + 0xed, 128, value); }
