/* Execute the actual reader against packed fixtures and a raw-bit oracle. */
#include <assert.h>
#include <stdint.h>
#include "../../src/record70.c"
const struct Record70 gRecords70[] = {
    {0},
    {
        .p00 = 65535,
        .p01 = 7,
        .p02 = 7,
        .p03 = 63,
        .p04 = 7,
        .p07 = 7,
        .p08 = 65535,
        .p09 = 255,
        .p10 = 65535,
        .p11 = 255,
        .p12 = 65535,
        .p13 = 255,
        .p14 = 7,
        .p15 = 255,
        .p16 = 255,
        .p17 = 7,
        .p18 = 255,
        .p25 = 65535,
        .p26 = 65535,
        .p27 = 65535,
        .p28 = 65535,
        .p29 = 255,
        .p30 = 255,
        .p31 = 255,
        .p33 = 255,
        .p34 = 255,
        .p35 = 255,
        .p36 = 255,
        .p37 = 255,
        .p38 = 255,
        .p39 = 255,
        .p40 = 255,
        .p41 = 255,
        .p42 = 255,
        .p44 = 255,
        .p46 = 15,
        .p47 = 127,
        .p48 = 63,
        .p49 = 15,
        .p50 = 127,
        .p51 = 63,
        .p52 = 63,
        .p53 = 255,
        .p54 = 255,
        .p57 = 255,
        .p58 = 255,
        .p59 = 65535,
        .p60 = 255,
        .flags = 65535,
        .p19 = {1, 2, 3, 4}, .p21 = {5, 6, 7, 8},
        .p22 = {9, 10, 11, 12}, .p23 = {13, 14, 15, 16},
    },
    {
        .p00 = 85,
        .p01 = 2,
        .p02 = 7,
        .p03 = 4,
        .p04 = 1,
        .p07 = 0,
        .p08 = 381,
        .p09 = 162,
        .p10 = 455,
        .p11 = 236,
        .p12 = 529,
        .p13 = 54,
        .p14 = 3,
        .p15 = 128,
        .p16 = 165,
        .p17 = 2,
        .p18 = 239,
        .p25 = 1010,
        .p26 = 1047,
        .p27 = 1084,
        .p28 = 1121,
        .p29 = 134,
        .p30 = 171,
        .p31 = 208,
        .p33 = 26,
        .p34 = 63,
        .p35 = 100,
        .p36 = 137,
        .p37 = 174,
        .p38 = 211,
        .p39 = 248,
        .p40 = 29,
        .p41 = 66,
        .p42 = 103,
        .p44 = 177,
        .p46 = 11,
        .p47 = 32,
        .p48 = 5,
        .p49 = 10,
        .p50 = 15,
        .p51 = 52,
        .p52 = 25,
        .p53 = 254,
        .p54 = 35,
        .p57 = 146,
        .p58 = 183,
        .p59 = 2268,
        .p60 = 1,
        .flags = 341,
        .p19 = {1, 2, 3, 4}, .p21 = {5, 6, 7, 8},
        .p22 = {9, 10, 11, 12}, .p23 = {13, 14, 15, 16},
    },
    {
        .p00 = 170,
        .p01 = 7,
        .p02 = 4,
        .p03 = 25,
        .p04 = 6,
        .p07 = 5,
        .p08 = 466,
        .p09 = 247,
        .p10 = 540,
        .p11 = 65,
        .p12 = 614,
        .p13 = 139,
        .p14 = 0,
        .p15 = 213,
        .p16 = 250,
        .p17 = 7,
        .p18 = 68,
        .p25 = 1095,
        .p26 = 1132,
        .p27 = 1169,
        .p28 = 1206,
        .p29 = 219,
        .p30 = 0,
        .p31 = 37,
        .p33 = 111,
        .p34 = 148,
        .p35 = 185,
        .p36 = 222,
        .p37 = 3,
        .p38 = 40,
        .p39 = 77,
        .p40 = 114,
        .p41 = 151,
        .p42 = 188,
        .p44 = 6,
        .p46 = 0,
        .p47 = 117,
        .p48 = 26,
        .p49 = 15,
        .p50 = 100,
        .p51 = 9,
        .p52 = 46,
        .p53 = 83,
        .p54 = 120,
        .p57 = 231,
        .p58 = 12,
        .p59 = 2353,
        .p60 = 86,
        .flags = 682,
        .p19 = {1, 2, 3, 4}, .p21 = {5, 6, 7, 8},
        .p22 = {9, 10, 11, 12}, .p23 = {13, 14, 15, 16},
    },
};
#include "../../src/record_decode.c"
static u32 raw_bits(const u8 *raw, unsigned start, unsigned width)
{
    u32 value = 0;
    unsigned bit;
    for (bit = 0; bit < width; bit++)
        value |= ((raw[(start + bit) / 8] >> ((start + bit) % 8)) & 1u) << bit;
    return value;
}
int main(void)
{
    static const struct { unsigned start, width; int pointer_offset; } fields[65] = {
        {0, 16, -1}, /* selector 0 */
        {16, 3, -1}, /* selector 1 */
        {19, 3, -1}, /* selector 2 */
        {22, 6, -1}, /* selector 3 */
        {28, 3, -1}, /* selector 4 */
        {520, 1, -1}, /* selector 5 */
        {521, 1, -1}, /* selector 6 */
        {31, 3, -1}, /* selector 7 */
        {40, 16, -1}, /* selector 8 */
        {56, 8, -1}, /* selector 9 */
        {64, 16, -1}, /* selector 10 */
        {80, 8, -1}, /* selector 11 */
        {88, 16, -1}, /* selector 12 */
        {104, 8, -1}, /* selector 13 */
        {112, 3, -1}, /* selector 14 */
        {115, 8, -1}, /* selector 15 */
        {123, 8, -1}, /* selector 16 */
        {131, 3, -1}, /* selector 17 */
        {134, 8, -1}, /* selector 18 */
        {0, 0, 18}, /* selector 19 */
        {0, 0, 18}, /* selector 20 */
        {0, 0, 22}, /* selector 21 */
        {0, 0, 26}, /* selector 22 */
        {0, 0, 30}, /* selector 23 */
        {0, 0, 34}, /* selector 24 */
        {272, 16, -1}, /* selector 25 */
        {288, 16, -1}, /* selector 26 */
        {304, 16, -1}, /* selector 27 */
        {320, 16, -1}, /* selector 28 */
        {424, 8, -1}, /* selector 29 */
        {408, 8, -1}, /* selector 30 */
        {416, 8, -1}, /* selector 31 */
        {0, 0, 42}, /* selector 32 */
        {336, 8, -1}, /* selector 33 */
        {344, 8, -1}, /* selector 34 */
        {352, 8, -1}, /* selector 35 */
        {360, 8, -1}, /* selector 36 */
        {368, 8, -1}, /* selector 37 */
        {376, 8, -1}, /* selector 38 */
        {384, 8, -1}, /* selector 39 */
        {392, 8, -1}, /* selector 40 */
        {400, 8, -1}, /* selector 41 */
        {432, 8, -1}, /* selector 42 */
        {524, 1, -1}, /* selector 43 */
        {440, 8, -1}, /* selector 44 */
        {525, 1, -1}, /* selector 45 */
        {448, 4, -1}, /* selector 46 */
        {452, 7, -1}, /* selector 47 */
        {459, 6, -1}, /* selector 48 */
        {465, 4, -1}, /* selector 49 */
        {469, 7, -1}, /* selector 50 */
        {476, 6, -1}, /* selector 51 */
        {482, 6, -1}, /* selector 52 */
        {488, 8, -1}, /* selector 53 */
        {496, 8, -1}, /* selector 54 */
        {522, 1, -1}, /* selector 55 */
        {523, 1, -1}, /* selector 56 */
        {504, 8, -1}, /* selector 57 */
        {512, 8, -1}, /* selector 58 */
        {536, 16, -1}, /* selector 59 */
        {552, 8, -1}, /* selector 60 */
        {526, 1, -1}, /* selector 61 */
        {527, 1, -1}, /* selector 62 */
        {528, 1, -1}, /* selector 63 */
        {529, 1, -1}, /* selector 64 */
    };
    unsigned record, selector;
    for (selector = 0; selector < 256; selector++)
        assert(DecodeRecordByte(selector) == (selector ? selector + 375 : 0));
    for (record = 0; record < sizeof(gRecords70) / sizeof(gRecords70[0]); record++) {
        const u8 *raw = (const u8 *)&gRecords70[record];
        for (selector = 0; selector < 256; selector++) {
            u32 expected = 0;
            unsigned decoded = selector == 42 || selector == 44 || selector == 53;
            if (selector < 65) {
                if (fields[selector].pointer_offset >= 0)
                    expected = (u32)(uintptr_t)(raw + fields[selector].pointer_offset);
                else expected = raw_bits(raw, fields[selector].start, fields[selector].width);
                if (selector == 30 || selector == 54) expected *= 200;
                if (decoded && expected) expected += 375;
            }
            assert(GetRecord70Property(record, selector) == expected);
        }
    }
    return 0;
}
