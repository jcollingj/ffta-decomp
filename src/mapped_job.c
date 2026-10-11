#include "job.h"
extern const u8 gMappedJobs[];
extern const void * const gContentPointers[];
u8 ReadMappedJob(u8 index) { return gMappedJobs[index]; }
u8 ReadMappedJobProperty01(u8 index)
{
    u8 job = gMappedJobs[index];
    return GetJobProperty(job, job, 1);
}
u8 ReadMappedJobProperty04(u8 index)
{
    u8 job = gMappedJobs[index];
    return GetJobProperty(job, job, 4);
}
u8 ReadMappedJobProperty06(u8 index)
{
    u8 job = gMappedJobs[index];
    return GetJobProperty(job, job, 6);
}
u8 ReadMappedJobProperty09(u8 index)
{
    u8 job = gMappedJobs[index];
    return GetJobProperty(job, job, 9);
}
const void *ReadMappedJobContentPointer(u8 index)
{
    const void * const *table = gContentPointers;
    u8 job = gMappedJobs[index];
    return table[GetJobProperty(job, job, 0)];
}
