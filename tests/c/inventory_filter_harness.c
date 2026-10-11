#include <assert.h>
#include <string.h>
#include "../../src/item.c"
#include "../../src/inventory_partition.c"
#include "../../src/inventory_class.c"
#include "../../src/inventory_filter.c"
struct InventoryEntry gInventoryEntries[375];
const struct Item gItems[376] = {
    [1] = {.properties03 = {0,0,0,3}},
    [2] = {.properties03 = {0,0,0,4}},
    [3] = {.properties03 = {29,0,0,5}},
    [253] = {.properties03 = {20,0,0,3}},
    [254] = {.properties03 = {20,0,0,4}},
    [255] = {.properties03 = {20,0,0,5}},
    [256] = {.properties03 = {29,0,0,0}},
    [257] = {.properties03 = {28,0,0,0}},
    [258] = {.properties03 = {0,0,0,5}},
    [362] = {.properties03 = {20,0,0,0}},
    [363] = {.properties03 = {29,0,0,0}},
    [364] = {.properties03 = {0,0,0,3}}
};
u8 ReadSaveByte(u16 index) { (void)index; assert(0); return 0; }
static unsigned ExpectedClass(unsigned item)
{
    switch (item) {
    case 1: case 253: case 364: return 0;
    case 2: case 254: return 1;
    case 255: case 362: return 3;
    case 256: case 257: case 258: case 363: return 4;
    }
    if (item >= 1 && item <= 252) return 2;
    if (item >= 362 && item <= 375) return 5;
    return 255;
}
int main(void)
{
    unsigned value, i, category, pass, start, length, count;
    int partition_count;
    struct InventoryEntry output[377], expected[377], before[375];
    for (value = 0; value <= 65535; value++) {
        int special = value >= 362 && value <= 375;
        assert(IsItemInRange1To252(value) == (value >= 1 && value <= 252));
        assert(ClassifyValueByTens(value) == (value < 10 ? 0 : value < 20 ? 1 : 2));
        assert(GetInventoryPartition(value, &partition_count) == gInventoryEntries + (special ? 361 : 0));
        assert(partition_count == (special ? 14 : 361));
        assert(GetInventoryPartition(value, 0) == gInventoryEntries + (special ? 361 : 0));
    }
    for (value = 0; value < 376; value++) assert(GetInventoryItemClass(value) == ExpectedClass(value));
    for (pass = 0; pass < 4; pass++) {
        for (i = 0; i < 375; i++) {
            gInventoryEntries[i].item = i + 1;
            gInventoryEntries[i].quantity = pass == 0 ? 0 : pass == 1 ? 255 : (i + pass) % 3 ? 1 : 0;
            gInventoryEntries[i].unknown = i ^ 0xa5;
        }
        memcpy(before, gInventoryEntries, sizeof(before));
        for (category = 0; category < 256; category++) {
            memset(output, 0xa5, sizeof(output));
            memcpy(expected, output, sizeof(output));
            start = category == 5 ? 361 : 0;
            length = category == 5 ? 14 : 361;
            count = 0;
            for (i = start; i < start + length; i++)
                if (ExpectedClass(i + 1) == category && before[i].quantity)
                    expected[1 + count++] = before[i];
            assert(CopyInventoryClass(output + 1, category) == count);
            assert(memcmp(output, expected, sizeof(output)) == 0);
        }
        for (value = 1; value <= 375; value++) {
            unsigned index = value > 361 ? value - 362 : value - 1;
            assert(FindInventoryItemIndex(value) == (before[value - 1].quantity ? index : 65535));
        }
        assert(FindInventoryItemIndex(0) == 65535);
        assert(FindInventoryItemIndex(65535) == 65535);
        assert(memcmp(before, gInventoryEntries, sizeof(before)) == 0);
    }
    memset(gInventoryEntries, 0, sizeof(gInventoryEntries));
    gInventoryEntries[2].item = gInventoryEntries[9].item = 17;
    gInventoryEntries[9].quantity = 1;
    assert(FindInventoryItemIndex(17) == 9);
    gInventoryEntries[2].quantity = 255;
    assert(FindInventoryItemIndex(17) == 2);
    gInventoryEntries[3].quantity = 1;
    assert(FindInventoryItemIndex(0) == 3);
    return 0;
}
