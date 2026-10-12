#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "../../src/nonmatching/collection_slots.c"

/* Oversized const fixtures keep every u16 probe within host memory while
 * preserving a visible value at each documented branch boundary. */
const u8 gClassRecords4[65536][4] = {
    [0] = { 0xC1, 0x11, 0x12, 0x13 },
    [1] = { 0xC2, 0x21, 0x22, 0x23 },
    [2] = { 0xC3, 0x31, 0x32, 0x33 },
    [65364] = { 0xD4, 0x41, 0x42, 0x43 },
};
const u8 gClassRecords16[65536][16] = {
    [0] = { 0x10 },
    [1] = { 0x21 },
    [84] = { 0x84 },
    [85] = { 0x85 },
    [86] = { 0x86 },
};

static uint32_t checksum(const volatile u8 *bytes, size_t length)
{
    uint32_t value = 2166136261u;
    size_t i;
    for (i = 0; i < length; i++) value = (value ^ bytes[i]) * 16777619u;
    return value;
}

static u8 expected(u16 input)
{
    if (input > 170) return gClassRecords4[input - 171][0];
    if (input > 85) input -= 85;
    return gClassRecords16[input][0];
}

int main(void)
{
    const uint32_t before4 = checksum(&gClassRecords4[0][0], sizeof(gClassRecords4));
    const uint32_t before16 = checksum(&gClassRecords16[0][0], sizeof(gClassRecords16));
    unsigned i;

    /* Independent branch vectors expose both sides of each threshold. */
    assert(ReadCollectionClass(0) == 0x10);
    assert(ReadCollectionClass(84) == 0x84);
    assert(ReadCollectionClass(85) == 0x85);
    assert(ReadCollectionClass(86) == 0x21);
    assert(ReadCollectionClass(170) == 0x85);
    assert(ReadCollectionClass(171) == 0xC1);
    assert(ReadCollectionClass(172) == 0xC2);
    assert(ReadCollectionClass(65535) == 0xD4);

    /* Exhaust all ABI u16 values; this is a safe host fixture sweep, not a
     * claim that the original game's legal collection domain is 0..65535. */
    for (i = 0; i <= 65535; i++)
        assert(ReadCollectionClass((u16)i) == expected((u16)i));

    assert(checksum(&gClassRecords4[0][0], sizeof(gClassRecords4)) == before4);
    assert(checksum(&gClassRecords16[0][0], sizeof(gClassRecords16)) == before16);
    return 0;
}
