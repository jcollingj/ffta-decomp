#include "unit.h"

u32 ReadUnitField(const struct UnitFieldView *unit, u8 field)
{
    switch (field) {
    case 0: return unit->word00;
    case 1: return unit->bytes04[0];
    case 2: return unit->bytes04[1];
    case 3: return unit->bytes04[2];
    case 4: return unit->bytes04[3];
    case 5: return unit->bytes04[4];
    case 6: return unit->bytes04[5];
    case 7: return unit->bytes04[6];
    case 8: return unit->bytes04[7];
    case 9: return (u32)&unit->bytes04[8];
    case 10: return unit->bytes04[8];
    case 11: return unit->bytes04[9];
    case 12: return unit->bytes04[10];
    case 13: return unit->bytes04[11];
    case 14: return unit->bytes04[12];
    case 15: return unit->bytes04[13];
    case 16: return unit->bytes04[14];
    case 17: return unit->bytes04[15];
    case 18: return unit->bytes04[16];
    case 19: return unit->half18[0];
    case 20: return unit->half18[1];
    case 21: return unit->half18[2];
    case 22: return unit->half18[3];
    case 23: return unit->half18[4];
    case 24: return unit->half18[5];
    case 25: return unit->half18[6];
    case 26: return unit->half18[7];
    case 27: return unit->half18[8];
    case 28: return (u32)&unit->half18[9];
    case 29: return unit->half18[9];
    case 30: return unit->half18[10];
    case 31: return unit->half18[11];
    case 32: return unit->half18[12];
    case 33: return unit->half18[13];
    case 34: return (u32)unit->unknown34;
    case 35: return unit->signedD0[0];
    case 36: return unit->signedD0[1];
    case 37: return unit->signedD0[2];
    case 38: return unit->halfD6;
    case 39: return (u32)unit->bytesD8;
    case 40: return unit->bytesD8[0];
    case 41: return unit->bytesD8[1];
    case 42: return unit->bytesD8[2];
    case 43: return unit->bytesD8[3];
    case 44: return unit->bytesD8[4];
    case 45: return unit->bytesD8[5];
    case 46: return unit->bytesD8[6];
    case 47: return unit->bytesD8[7];
    case 48: return unit->bytesD8[8];
    case 49: return unit->bytesD8[9];
    case 50: return unit->bytesD8[10];
    case 51: return unit->bytesD8[11];
    case 52: return unit->bytesD8[12];
    case 53: return unit->bytesD8[13];
    case 54: return unit->bytesD8[14];
    case 55: return unit->bytesD8[15];
    case 56: return (u32)unit->unknownE8;
    case 57: return unit->bytesF1[0];
    case 58: return unit->bytesF1[1];
    case 59: return unit->bytesF1[2];
    case 60: return unit->bytesF1[3];
    case 61: return unit->bytesF1[4];
    case 62: return unit->bytesF1[5];
    case 63: return unit->bytesF1[6];
    case 64: return unit->bytesF1[7];
    case 65: return unit->bytesF1[8];
    case 66: return unit->bytesF1[9];
    case 67: return unit->bytesF1[10];
    case 68: return (u32)unit->tailFC;
    default: return 0;
    }
}

u8 UnitField01AboveOne(const struct UnitFieldView *unit)
{
    u8 result = 0;
    if (unit && unit->bytes04[0] > 1) result = 1;
    return result;
}

u8 UnitField01Equals(const struct UnitFieldView *unit, u8 value)
{
    u8 result = 0;
    if (unit && unit->bytes04[0] == value) result = 1;
    return result;
}

u8 UnitField03Equals1(const struct UnitFieldView *unit)
{
    u8 result = 0;
    if (unit && unit->bytes04[2] == 1) result = 1;
    return result;
}

u8 UnitField03Equals2(const struct UnitFieldView *unit)
{
    u8 result = 0;
    if (unit && unit->bytes04[2] == 2) result = 1;
    return result;
}

u8 UnitField03Equals3(const struct UnitFieldView *unit)
{
    u8 result = 0;
    if (unit && unit->bytes04[2] == 3) result = 1;
    return result;
}

u8 UnitField03Equals4(const struct UnitFieldView *unit)
{
    u8 result = 0;
    if (unit && unit->bytes04[2] == 4) result = 1;
    return result;
}

u8 UnitField03Equals5(const struct UnitFieldView *unit)
{
    u8 result = 0;
    if (unit && unit->bytes04[2] == 5) result = 1;
    return result;
}

u8 UnitField04Equals24(const struct UnitFieldView *unit)
{
    u8 result = 0;
    if (unit && unit->bytes04[3] == 24) result = 1;
    return result;
}

u8 UnitField27HasBit15(const struct UnitFieldView *unit)
{
    u8 result = 0;
    if (unit) {
        u16 bits = unit->half18[8] & 0x8000;
        result = bits != 0;
    }
    return result;
}

u8 UnitField27HasBit13(const struct UnitFieldView *unit)
{
    u8 result = 0;
    if (unit) {
        u16 bits = unit->half18[8] & 0x2000;
        result = bits != 0;
    }
    return result;
}

u8 UnitField19IsZero(const struct UnitFieldView *unit)
{
    u8 result = 0;
    if (unit && unit->half18[0] == 0) result = 1;
    return result;
}

u8 UnitField27HasBit12(const struct UnitFieldView *unit)
{
    u8 result = 0;
    if (unit) {
        u16 bits = unit->half18[8] & 0x1000;
        result = bits != 0;
    }
    return result;
}

u8 UnitField27HasBit14(const struct UnitFieldView *unit)
{
    u8 result = 0;
    if (unit) {
        u16 bits = unit->half18[8] & 0x4000;
        result = bits != 0;
    }
    return result;
}

u8 UnitField27HasBit3(const struct UnitFieldView *unit)
{
    u8 result = 0;
    if (unit) {
        u16 bits = unit->half18[8] & 0x8;
        result = bits != 0;
    }
    return result;
}

u8 UnitField27HasBit4(const struct UnitFieldView *unit)
{
    u8 result = 0;
    if (unit) {
        u16 bits = unit->half18[8] & 0x10;
        result = bits != 0;
    }
    return result;
}

u8 UnitField27HasBit2(const struct UnitFieldView *unit)
{
    u8 result = 0;
    if (unit) {
        u16 bits = unit->half18[8] & 0x4;
        result = bits != 0;
    }
    return result;
}

u8 UnitField27HasBit6(const struct UnitFieldView *unit)
{
    u8 result = 0;
    if (unit) {
        u16 bits = unit->half18[8] & 0x40;
        result = bits != 0;
    }
    return result;
}

u8 UnitField27HasBit5(const struct UnitFieldView *unit)
{
    u8 result = 0;
    if (unit) {
        u16 bits = unit->half18[8] & 0x20;
        result = bits != 0;
    }
    return result;
}

u8 UnitField27HasBit8(const struct UnitFieldView *unit)
{
    u8 result = 0;
    if (unit) {
        u16 bits = unit->half18[8] & 0x100;
        result = bits != 0;
    }
    return result;
}
u8 UnitField03InRange6To18(const struct UnitFieldView *unit)
{
    u8 value;
    u8 result;
    if (!unit) return 0;
    value = unit->bytes04[2];
    result = 0;
    if ((u8)(value - 6) <= 12) result = 1;
    return result;
}
