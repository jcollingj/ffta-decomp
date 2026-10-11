#ifndef FFTA_RECORD70_H
#define FFTA_RECORD70_H
#include "types.h"
/* Partial format; complete table extent and gameplay field names are unknown. */
struct Record70 {
    u16 p00;
    u32 p01:3, p02:3, p03:6, p04:3, p07:3, unknown04:6;
    u16 p08;
    u8 p09;
    u16 p10;
    u8 p11;
    u16 p12;
    u8 p13;
    u32 p14:3, p15:8, p16:8, p17:3, p18:8, unknown17:2;
    u8 p19[4], p21[4], p22[4], p23[4];
    u16 p25, p26, p27, p28;
    u8 p33, p34, p35, p36, p37, p38, p39, p40, p41;
    u8 p30, p31, p29, p42, p44;
    u32 p46:4, p47:7, p48:6, p49:4, p50:7, p51:6, p52:6;
    u8 p53, p54, p57, p58;
    u16 flags;
    u16 p59;
    u8 p60;
} __attribute__((packed));
typedef char Record70Size[(sizeof(struct Record70) == 70) ? 1 : -1];
extern const struct Record70 gRecords70[];
u16 DecodeRecordByte(u8 value);
u32 GetRecord70Property(u16 index, u8 property);
#endif
