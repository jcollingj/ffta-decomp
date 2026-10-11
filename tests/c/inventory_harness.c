/* Execute the recovered source directly. ARM byte comparison remains a separate
 * gate; this host test only covers portable inventory arithmetic/state changes. */
#include <assert.h>
#include <string.h>
#include "../../src/inventory.c"

struct InventoryEntry gInventoryEntries[375];
static void reset(void) { memset(gInventoryEntries, 0, sizeof(gInventoryEntries)); }

int main(void)
{
    int i;
    assert(sizeof(u16) == 2 && sizeof(u8) == 1);
    reset();
    assert(AddInventoryItem(0, 1) == -1);
    assert(AddInventoryItem(1, 0) == -1);
    assert(RemoveInventoryItem(0, 0) == 0);
    assert(RemoveInventoryItem(0, 1) == -1);
    assert(gInventoryEntries[0].item == 0);

    /* Partition endpoints and the first free slot. */
    assert(AddInventoryItem(361, 2) == 0);
    assert(gInventoryEntries[0].item == 361);
    assert(AddInventoryItem(362, 3) == 0);
    assert(gInventoryEntries[361].item == 362);
    assert(AddInventoryItem(375, 4) == 0);
    assert(gInventoryEntries[362].item == 375);
    assert(AddInventoryItem(362, 2) == 0);
    assert(gInventoryEntries[361].quantity == 5);

    /* Removal keeps the item ID even after its quantity reaches zero. */
    assert(RemoveInventoryItem(362, 200) == 0);
    assert(gInventoryEntries[361].quantity == 0);
    assert(gInventoryEntries[361].item == 362);
    assert(RemoveInventoryItem(363, 1) == -1);

    reset();
    assert(AddInventoryItem(1, 98) == 0);
    assert(AddInventoryItem(1, 3) == 2);
    assert(gInventoryEntries[0].quantity == 99);
    assert(AddInventoryItem(1, 200) == 0); /* 299 wraps to 43 before the cap. */
    assert(gInventoryEntries[0].quantity == 43);
    assert(RemoveInventoryItem(1, 4) == 0);
    assert(gInventoryEntries[0].quantity == 39);
    gInventoryEntries[0].quantity = 99;
    assert(AddInventoryItem(1, 150) == -106); /* 150 excess narrows to signed byte. */
    assert(gInventoryEntries[0].quantity == 99);

    reset();
    for (i = 0; i < 361; i++) {
        gInventoryEntries[i].item = 1;
        gInventoryEntries[i].quantity = 1;
    }
    assert(AddInventoryItem(2, 1) == -1); /* Must not consume the other pool. */
    assert(gInventoryEntries[361].item == 0);
    assert(AddInventoryItem(362, 1) == 0);
    assert(AddInventoryItem(1, 1) == 0); /* Existing entry still works when full. */
    assert(gInventoryEntries[0].quantity == 2);
    assert(gInventoryEntries[1].quantity == 1);
    return 0;
}
