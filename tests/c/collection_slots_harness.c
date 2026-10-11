#include <assert.h>
#include <string.h>
#include "../../src/collection_slots.c"
u8 gCollectionBytes[20];
u16 gCollectionHalfwords[20];
int main(void)
{
    unsigned count, i, hole, other;
    u16 expected[20];
    for (count = 0; count <= 20; count++) {
        for (i = 0; i < 20; i++) {
            /* Permutation spreads occupied entries across the full range. */
            unsigned slot = i * 7 % 20;
            gCollectionBytes[slot] = i < count ? 255 : 0;
            gCollectionHalfwords[slot] = i < count ? 65535 : 0;
        }
        memcpy(expected, gCollectionHalfwords, sizeof(expected));
        assert(CountCollectionBytes() == count);
        assert(CountCollectionHalfwords() == count);
        assert(memcmp(expected, gCollectionHalfwords, sizeof(expected)) == 0);
    }
    for (hole = 0; hole < 20; hole++) {
        for (other = hole; other < 20; other++) {
            for (i = 0; i < 20; i++) gCollectionHalfwords[i] = i + 1;
            gCollectionHalfwords[hole] = gCollectionHalfwords[other] = 0;
            memcpy(expected, gCollectionHalfwords, sizeof(expected));
            AddCollectionHalfword(0);
            assert(memcmp(expected, gCollectionHalfwords, sizeof(expected)) == 0);
            expected[hole] = 65535;
            AddCollectionHalfword(65535);
            assert(memcmp(expected, gCollectionHalfwords, sizeof(expected)) == 0);
        }
    }
    memset(gCollectionHalfwords, 0, sizeof(gCollectionHalfwords));
    for (count = 0; count < 20; count++) {
        AddCollectionHalfword(7);
        assert(CountCollectionHalfwords() == count + 1);
        for (i = 0; i < 20; i++) assert(gCollectionHalfwords[i] == (i <= count ? 7 : 0));
    }
    memcpy(expected, gCollectionHalfwords, sizeof(expected));
    AddCollectionHalfword(1234);
    assert(memcmp(expected, gCollectionHalfwords, sizeof(expected)) == 0);
    return 0;
}
