#include "job.h"

/* Preserve the original inheritance behavior. A value of 255 follows the
 * caller's fallback job; any other nonzero value selects another record.
 * Invalid selectors and cyclic inheritance loop indefinitely in the original.
 * Each switch arm resolves inheritance before reading its packed field.
 */
#define READ_PROPERTY(selector, field) \
    case selector: \
        if (job->inherit == 255) \
            job = &gJobs[fallback_job]; \
        else if (job->inherit != 0) \
            job = &gJobs[job->inherit]; \
        else \
            return job->field; \
        break

u32 GetJobProperty(u8 job_id, u8 fallback_job, u8 property)
{
    const struct Job *job = &gJobs[job_id];
    for (;;) {
        switch (property) {
        READ_PROPERTY(0, p00);
        READ_PROPERTY(1, p01);
        case 2: return job->inherit;
        READ_PROPERTY(3, p03);
        READ_PROPERTY(4, p04);
        READ_PROPERTY(5, p05);
        READ_PROPERTY(6, p06);
        READ_PROPERTY(7, p07);
        READ_PROPERTY(8, p08);
        READ_PROPERTY(9, p09);
        READ_PROPERTY(10, p10);
        READ_PROPERTY(11, p11);
        READ_PROPERTY(12, p12);
        READ_PROPERTY(13, p13);
        READ_PROPERTY(14, p14);
        READ_PROPERTY(15, p15);
        READ_PROPERTY(16, p16);
        READ_PROPERTY(17, p17);
        READ_PROPERTY(18, p18);
        READ_PROPERTY(19, p19);
        READ_PROPERTY(20, p20);
        READ_PROPERTY(21, p21);
        READ_PROPERTY(22, p22);
        READ_PROPERTY(23, p23);
        READ_PROPERTY(24, p24);
        READ_PROPERTY(25, p25);
        READ_PROPERTY(26, p26);
        READ_PROPERTY(27, p27);
        READ_PROPERTY(28, p28);
        READ_PROPERTY(29, p29);
        READ_PROPERTY(30, p30);
        READ_PROPERTY(31, p31);
        READ_PROPERTY(32, p32);
        READ_PROPERTY(34, p34);
        READ_PROPERTY(35, p35);
        READ_PROPERTY(33, p33);
        READ_PROPERTY(36, p36);
        READ_PROPERTY(37, p37);
        READ_PROPERTY(38, p38);
        READ_PROPERTY(39, p39);
        READ_PROPERTY(40, p40);
        READ_PROPERTY(41, p41);
        READ_PROPERTY(42, p42);
        READ_PROPERTY(43, p43);
        READ_PROPERTY(44, p44);
        READ_PROPERTY(45, p45);
        READ_PROPERTY(46, p46);
        READ_PROPERTY(47, p47);
        }
    }
}

#undef READ_PROPERTY
