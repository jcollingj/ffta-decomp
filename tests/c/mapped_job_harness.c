#include <assert.h>
#include "../../src/job_property.c"
#include "../../src/mapped_job.c"
/* Full byte-index fixture; no assertion about the original mapping's extent. */
const u8 gMappedJobs[256] = {
    0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
    0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
    0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
    0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
    0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
    0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
    0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
    0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
    0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
    0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
    0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
    0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
    0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
    0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
    0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
    0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
};
const struct Job gJobs[] = {
    {.p00=0, .p01=0, .p04=0x100, .p06=0, .p09=0},
    {.p00=1, .p01=4, .p04=0x1ff, .p06=15, .p09=255},
    {.p00=2, .p01=5, .p04=0x1234, .p06=7, .p09=128},
    {.p00=3, .p01=99, .p04=0xffff, .p06=1, .p09=1, .inherit=1},
};
static const u8 tokens[4] = {0};
const void * const gContentPointers[] = {tokens, tokens+1, tokens+2, tokens+3};
int main(void)
{
    unsigned index;
    for (index = 0; index < 256; index++) {
        unsigned mapped = index % 4;
        const struct Job *effective = &gJobs[mapped == 3 ? 1 : mapped];
        assert(ReadMappedJob(index) == mapped);
        assert(ReadMappedJobProperty01(index) == effective->p01);
        assert(ReadMappedJobProperty04(index) == (effective->p04 & 255));
        assert(ReadMappedJobProperty06(index) == effective->p06);
        assert(ReadMappedJobProperty09(index) == effective->p09);
        assert(ReadMappedJobContentPointer(index) == tokens + effective->p00);
    }
    return 0;
}
