#ifndef FFTA_UNIT_H
#define FFTA_UNIT_H
#include "types.h"
/* Partial in-memory layout proven by ReadUnitField. Names describe offsets,
 * not inferred gameplay semantics. The object continues beyond offset 0xFC. */
struct UnitFieldView {
    u32 word00;
    u8 bytes04[20];
    u16 half18[14];
    u8 unknown34[156];
    signed short signedD0[3];
    u16 halfD6;
    u8 bytesD8[16];
    u8 unknownE8[9];
    u8 bytesF1[11];
    u8 tailFC[0];
};
typedef char UnitFieldViewSize[(sizeof(struct UnitFieldView) == 0xfc) ? 1 : -1];
u32 ReadUnitField(const struct UnitFieldView *, u8);
u8 UnitField01AboveOne(const struct UnitFieldView *unit);
u8 UnitField01Equals(const struct UnitFieldView *unit, u8 value);
u8 UnitField03Equals1(const struct UnitFieldView *unit);
u8 UnitField03Equals2(const struct UnitFieldView *unit);
u8 UnitField03Equals3(const struct UnitFieldView *unit);
u8 UnitField03Equals4(const struct UnitFieldView *unit);
u8 UnitField03Equals5(const struct UnitFieldView *unit);
u8 UnitField04Equals24(const struct UnitFieldView *unit);
u8 UnitField27HasBit15(const struct UnitFieldView *unit);
u8 UnitField27HasBit13(const struct UnitFieldView *unit);
u8 UnitField19IsZero(const struct UnitFieldView *unit);
u8 UnitField27HasBit12(const struct UnitFieldView *unit);
u8 UnitField27HasBit14(const struct UnitFieldView *unit);
u8 UnitField27HasBit3(const struct UnitFieldView *unit);
u8 UnitField27HasBit4(const struct UnitFieldView *unit);
u8 UnitField27HasBit2(const struct UnitFieldView *unit);
u8 UnitField27HasBit6(const struct UnitFieldView *unit);
u8 UnitField27HasBit5(const struct UnitFieldView *unit);
u8 UnitField27HasBit8(const struct UnitFieldView *unit);
u8 UnitField03InRange6To18(const struct UnitFieldView *unit);
#endif
