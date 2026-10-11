/* Packed record reader at 0xCE4DC; gameplay role and table extent unverified. */
#include "record70.h"
u32 GetRecord70Property(u16 index, u8 property)
{
    const struct Record70 *record = &gRecords70[index];
    switch (property) {
    case 0: return record->p00;
    case 1: return record->p01;
    case 2: return record->p02;
    case 3: return record->p03;
    case 4: return record->p04;
    case 5: return (record->flags >> 0) & 1;
    case 6: return (u8)(record->flags >> 1) & 1;
    case 7: return record->p07;
    case 8: return record->p08;
    case 9: return record->p09;
    case 10: return record->p10;
    case 11: return record->p11;
    case 12: return record->p12;
    case 13: return record->p13;
    case 14: return record->p14;
    case 15: return record->p15;
    case 16: return record->p16;
    case 17: return record->p17;
    case 18: return record->p18;
    case 19: return (u32)&record->p19;
    case 20: return (u32)&record->p19;
    case 21: return (u32)&record->p21;
    case 22: return (u32)&record->p22;
    case 23: return (u32)&record->p23;
    case 24: return (u32)&record->p25;
    case 25: return record->p25;
    case 26: return record->p26;
    case 27: return record->p27;
    case 28: return record->p28;
    case 29: return record->p29;
    case 30: return record->p30 * 200;
    case 31: return record->p31;
    case 32: return (u32)&record->p33;
    case 33: return record->p33;
    case 34: return record->p34;
    case 35: return record->p35;
    case 36: return record->p36;
    case 37: return record->p37;
    case 38: return record->p38;
    case 39: return record->p39;
    case 40: return record->p40;
    case 41: return record->p41;
    case 42: return DecodeRecordByte(record->p42);
    case 43: return (u8)(record->flags >> 4) & 1;
    case 44: return DecodeRecordByte(record->p44);
    case 45: return (u8)(record->flags >> 5) & 1;
    case 46: return record->p46;
    case 47: return record->p47;
    case 48: return record->p48;
    case 49: return record->p49;
    case 50: return record->p50;
    case 51: return record->p51;
    case 52: return record->p52;
    case 53: return DecodeRecordByte(record->p53);
    case 54: return record->p54 * 200;
    case 55: return (u8)(record->flags >> 2) & 1;
    case 56: return (u8)(record->flags >> 3) & 1;
    case 57: return record->p57;
    case 58: return record->p58;
    case 59: return record->p59;
    case 60: return record->p60;
    case 61: return (u8)(record->flags >> 6) & 1;
    case 62: return (u8)(record->flags >> 7) & 1;
    case 63: return (u8)(record->flags >> 8) & 1;
    case 64: return (u8)(record->flags >> 9) & 1;
    default: return 0;
    }
}
asm(".align 2, 0");
