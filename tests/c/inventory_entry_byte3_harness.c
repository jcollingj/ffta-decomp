#include <assert.h>
#include <string.h>
#include "../../src/inventory_entry_byte3.c"
struct InventoryEntry gInventoryEntries[375];
int main(void)
{
    struct InventoryEntry expected[375];
    unsigned quantity,value,mode,i,item;
    static const u8 modes[]={0,1,128,255};
    for (quantity=0;quantity<256;quantity++) for (value=0;value<256;value++)
    for (mode=0;mode<4;mode++) {
        memset(gInventoryEntries,0,sizeof(gInventoryEntries));
        gInventoryEntries[0].item=17;
        gInventoryEntries[0].quantity=quantity;
        gInventoryEntries[0].unknown=value;
        memcpy(expected,gInventoryEntries,sizeof(expected));
        expected[0].unknown=modes[mode] ? (value<quantity ? value+1 : value) : (value ? value-1 : 0);
        AdjustInventoryEntryByte3(17,modes[mode]);
        assert(memcmp(expected,gInventoryEntries,sizeof(expected))==0);
    }
    for (i=0;i<375;i++) {
        memset(gInventoryEntries,0,sizeof(gInventoryEntries));
        item=i<361 ? 17 : 375;
        gInventoryEntries[i].item=item;
        gInventoryEntries[i].quantity=5;
        memcpy(expected,gInventoryEntries,sizeof(expected));
        expected[i].unknown=1;
        AdjustInventoryEntryByte3(item,1);
        assert(memcmp(expected,gInventoryEntries,sizeof(expected))==0);
    }
    memset(gInventoryEntries,0,sizeof(gInventoryEntries));
    gInventoryEntries[0].unknown=5;
    memcpy(expected,gInventoryEntries,sizeof(expected));
    AdjustInventoryEntryByte3(0,0);
    AdjustInventoryEntryByte3(17,1);
    AdjustInventoryEntryByte3(375,0);
    assert(memcmp(expected,gInventoryEntries,sizeof(expected))==0);
    /* Empty-quantity first duplicate wins; a later duplicate is not modified. */
    gInventoryEntries[0].item=17;
    gInventoryEntries[1].item=17;
    gInventoryEntries[1].quantity=10;
    memcpy(expected,gInventoryEntries,sizeof(expected));
    AdjustInventoryEntryByte3(17,255);
    assert(memcmp(expected,gInventoryEntries,sizeof(expected))==0);
    return 0;
}
