#include <assert.h>
#include <string.h>
#include "../../src/unit_pool.c"

struct UnitRecord gUnitPoolA[12];

int main(void)
{
    struct UnitRecord before[12];
    unsigned slot;
    memset(gUnitPoolA, 0xff, sizeof(gUnitPoolA));
    assert(FindEmptyUnitPoolASlot() == 0);
    for (slot = 0; slot < 12; slot++) {
        gUnitPoolA[slot].fields.bytes04[0] = 0;
        memcpy(before, gUnitPoolA, sizeof(before));
        assert(FindEmptyUnitPoolASlot() == &gUnitPoolA[slot]);
        assert(memcmp(before, gUnitPoolA, sizeof(before)) == 0);
        gUnitPoolA[slot].fields.bytes04[0] = 1;
    }
    /* Only the occupancy byte is relevant; free slots retain other data. */
    gUnitPoolA[3].fields.bytes04[0] = 0;
    gUnitPoolA[9].fields.bytes04[0] = 0;
    assert(FindEmptyUnitPoolASlot() == &gUnitPoolA[3]);
    return 0;
}
