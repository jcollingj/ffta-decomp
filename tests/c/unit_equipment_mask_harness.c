#include <assert.h>
#include <string.h>
#include "../../src/job_property.c"
#include "../../src/unit_property.c"
#include "../../src/item.c"
#include "../../src/unit_equipment_mask.c"
const u32 gEquipmentClassMasks[] = {0, 0xffffffffu, 0x55555555u, 0xaaaaaaaau};
const struct Job gJobs[] = {
    {.p36=0}, {.p36=1}, {.p36=2}, {.p36=3}, {.inherit=2}, {.inherit=255}
};
#define CATEGORY(n) [n] = {.properties03={n}}
const struct Item gItems[33] = {
    CATEGORY(1), CATEGORY(2), CATEGORY(3), CATEGORY(4), CATEGORY(5), CATEGORY(6),
    CATEGORY(7), CATEGORY(8), CATEGORY(9), CATEGORY(10), CATEGORY(11), CATEGORY(12),
    CATEGORY(13), CATEGORY(14), CATEGORY(15), CATEGORY(16), CATEGORY(17), CATEGORY(18),
    CATEGORY(19), CATEGORY(20), CATEGORY(21), CATEGORY(22), CATEGORY(23), CATEGORY(24),
    CATEGORY(25), CATEGORY(26), CATEGORY(27), CATEGORY(28), CATEGORY(29), CATEGORY(30),
    CATEGORY(31), CATEGORY(32)
};
u8 ReadSaveByte(u16 index) { (void)index; assert(0); return 0; }
static struct EquipmentView *expected_unit;
static unsigned expected_item, expected_slot, result, calls;
u32 InspectEquipment(struct EquipmentView *unit, u16 item, u8 slot, u8 mode)
{
    assert(unit == expected_unit && item == expected_item && slot == expected_slot);
    assert(mode == 0);
    calls++;
    return result;
}
int main(void)
{
    u8 unit[0x108], before[0x108];
    struct EquipmentView equipment;
    unsigned job, fallback, category, mask;
    static const unsigned results[] = {0,1,0x81,255,256,0x12345678u,0xffffffffu};
    unsigned value;
    for (job=0; job<6; job++) for (fallback=0; fallback<4; fallback++) {
        memset(unit, 0xa5, sizeof(unit));
        unit[5]=job;
        unit[7]=fallback;
        memcpy(before, unit, sizeof(unit));
        mask=job==4 ? 2 : job==5 ? fallback : job;
        for (category=1; category<=32; category++) {
            unsigned allowed = (gEquipmentClassMasks[mask] >> (category-1)) & 1;
            assert(UnitItemCategoryAllowed(unit, category) == allowed);
            assert(memcmp(before, unit, sizeof(unit)) == 0);
        }
    }
    expected_unit=&equipment;
    for (expected_slot=0; expected_slot<256; expected_slot++) {
        expected_item=expected_slot * 257;
        for (value=0; value<sizeof(results)/sizeof(results[0]); value++) {
            result=results[value]; calls=0;
            assert(InspectEquipmentMode0(&equipment, expected_item, expected_slot) == (result & 255));
            assert(calls==1);
        }
    }
    return 0;
}
