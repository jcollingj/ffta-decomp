#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "../../src/save_collection.c"
struct SaveCollectionView gSaveData;
int main(void)
{
    struct SaveCollectionView expected;
    unsigned count, i, value;
    assert(offsetof(struct SaveCollectionView, counter) == 0x1f68);
    assert(offsetof(struct SaveCollectionView, records) == 0x2e78);
    for (count = 0; count <= 20; count++) {
        memset(&gSaveData, 0xa5, sizeof(gSaveData));
        for (i = 0; i < 20; i++)
            gSaveData.records[i * 7 % 20].word00 = i < count ? (1u << (i % 32)) : 0;
        expected = gSaveData;
        assert(HasSaveCollectionRecord() == (count != 0));
        assert(CountSaveCollectionRecords() == count);
        assert(memcmp(&gSaveData, &expected, sizeof(expected)) == 0);
    }
    for (i = 0; i < 20; i++) {
        memset(&gSaveData, 0, sizeof(gSaveData));
        gSaveData.records[i].word00 = 0x80000000u;
        assert(HasSaveCollectionRecord() == 1);
        assert(CountSaveCollectionRecords() == 1);
    }
    for (value = 0; value < 256; value++) {
        memset(&gSaveData, 0xa5, sizeof(gSaveData));
        gSaveData.counter = value;
        expected = gSaveData;
        if (value < 99) expected.counter = value + 1;
        IncrementSaveCounterBelow99();
        assert(memcmp(&gSaveData, &expected, sizeof(expected)) == 0);
    }
    return 0;
}
