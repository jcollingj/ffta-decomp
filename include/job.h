#ifndef FFTA_JOB_H
#define FFTA_JOB_H
#include "types.h"
/* Packed 52-byte record at 0x08521A14. Names pNN correspond to selectors in
 * GetJobProperty; their gameplay meanings must be recovered from callers.
 */
struct Job {
    u16 p00;
    u8 unknown02[2];
    u8 p01;
    u8 inherit;
    u8 p03;
    u16 p04, p05;
    u32 p06:4;
    u32 p07:4;
    u8 p08, p09, p10, p11, p12, p13;
    u32 unknown18:3;
    u32 p14:3;
    u32 p15:3;
    u32 p16:3;
    u32 p17:3;
    u32 p18:3;
    u32 p19:3;
    u32 p20:3;
    u32 p21:3;
    u32 unknown21:5;
    u8 p22, p23, p24, p25;
    u32 p26:12;
    u32 p27:12;
    u32 p28:12;
    u32 p29:12;
    u8 p41, p42, p43, p44, p45, p46, p47;
    u8 unknown27;
    u8 p30, p31, p32;
    u32 p34:4;
    u32 p35:4;
    u8 p33, p36, p37, p38, p39, p40;
    u8 unknown32[2];
} __attribute__((packed));
typedef char JobSizeMustBe52[(sizeof(struct Job) == 52) ? 1 : -1];
extern const struct Job gJobs[];
u32 GetJobProperty(u8 job_id, u8 fallback_job, u8 property);
#endif
