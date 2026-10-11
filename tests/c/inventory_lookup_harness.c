#include <assert.h>
#include <string.h>
#include "../../src/inventory.c"
#include "../../src/inventory_lookup.c"
#include "../../src/special_item_mapping.c"
struct InventoryEntry gInventoryEntries[375];
int main(void)
{
    static const u16 mapping[15] = {0,362,363,364,365,366,375,369,370,371,373,372,367,374,368};
    unsigned i, quantity, other, item;
    struct InventoryEntry expected[375];
    for (i = 0; i < 256; i++) assert(MapSpecialItemIndex(i) == (i < 15 ? mapping[i] : 0));
    for (i = 0; i < 375; i++) {
        gInventoryEntries[i].item = i + 1;
        gInventoryEntries[i].quantity = i % 256;
        gInventoryEntries[i].unknown = (i * 17) % 256;
    }
    memcpy(expected, gInventoryEntries, sizeof(expected));
    for (item = 0; item < 65536; item++) {
        unsigned difference = 0, present = 0;
        if (item >= 1 && item <= 375) {
            difference = (expected[item-1].quantity - expected[item-1].unknown) & 255;
            present = expected[item-1].quantity != 0;
        }
        assert(GetInventoryQuantityDifference(item) == difference);
        assert(HasInventoryItem(item) == present);
    }
    assert(memcmp(expected, gInventoryEntries, sizeof(expected)) == 0);
    memset(gInventoryEntries, 0, sizeof(gInventoryEntries));
    gInventoryEntries[0].item = 17;
    for (quantity = 0; quantity < 256; quantity++)
        for (other = 0; other < 256; other++) {
            gInventoryEntries[0].quantity = quantity;
            gInventoryEntries[0].unknown = other;
            assert(GetInventoryQuantityDifference(17) == ((quantity - other) & 255));
            assert(HasInventoryItem(17) == (quantity != 0));
        }
    /* Difference returns the first ID even when empty; presence can find a later one. */
    gInventoryEntries[0].quantity = 0;
    gInventoryEntries[0].unknown = 7;
    gInventoryEntries[2].item = 17;
    gInventoryEntries[2].quantity = 5;
    assert(GetInventoryQuantityDifference(17) == 249);
    assert(HasInventoryItem(17) == 1);
    gInventoryEntries[3].quantity = 255;
    assert(GetInventoryQuantityDifference(0) == 0);
    assert(HasInventoryItem(0) == 0);
    /* Existing mutation routines and the recovered queries share both partitions. */
    for (i = 0; i < 2; i++) {
        memset(gInventoryEntries, 0, sizeof(gInventoryEntries));
        item = i ? MapSpecialItemIndex(6) : 17;
        assert(AddInventoryItem(item, 10) == 0);
        assert(HasInventoryItem(item) == 1);
        assert(GetInventoryQuantityDifference(item) == 10);
        assert(RemoveInventoryItem(item, 3) == 0);
        assert(GetInventoryQuantityDifference(item) == 7);
        assert(RemoveInventoryItem(item, 255) == 0);
        assert(HasInventoryItem(item) == 0);
        assert(GetInventoryQuantityDifference(item) == 0);
    }
    return 0;
}
