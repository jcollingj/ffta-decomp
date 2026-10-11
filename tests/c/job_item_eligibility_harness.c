#include <assert.h>
#include "../../src/job_item_eligibility.c"
const u32 gEquipmentClassMasks[] = {0, 0xffffffffu, 0x55555555u, 0xaaaaaaaau};
static unsigned test_job, test_item, group_value, category_value, mask_index;
static unsigned job_calls, item_calls;
u32 GetJobProperty(u8 job, u8 fallback, u8 property)
{
    assert(job == test_job && fallback == test_job);
    assert(property == (job_calls == 0 ? 1 : 36));
    job_calls++;
    return property == 1 ? group_value : mask_index;
}
u32 GetItemProperty(u16 item, u8 property)
{
    assert(item == test_item && property == 3);
    item_calls++;
    return category_value;
}
int main(void)
{
    static const u16 items[] = {0, 286, 287, 288, 310, 311, 312, 315, 316, 317, 65535};
    static const unsigned groups[] = {0, 3, 4, 5, 255, 260};
    unsigned category, group, item, mask, wide;
    for (category = 1; category <= 32; category++) {
        for (group = 0; group < sizeof(groups) / sizeof(groups[0]); group++) {
            for (item = 0; item < sizeof(items) / sizeof(items[0]); item++) {
                for (mask = 0; mask < 4; mask++) {
                    for (wide = 0; wide < 2; wide++) {
                        unsigned expected;
                        test_job = item * 19;
                        test_item = items[item];
                        group_value = groups[group];
                        category_value = category + wide * 256;
                        mask_index = mask;
                        job_calls = item_calls = 0;
                        expected = (gEquipmentClassMasks[mask] >> (category - 1)) & 1;
                        if (category == 22) expected = (group_value & 255) == 4;
                        else if ((category == 23 || category == 25) &&
                                 (test_item == 287 || test_item == 311 || test_item == 316) &&
                                 (group_value & 255) != 4) expected = 0;
                        assert(JobAllowsItem(test_job, test_item) == expected);
                        assert(job_calls == 2 && item_calls == 1);
                    }
                }
            }
        }
    }
    return 0;
}
