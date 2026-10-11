#include <assert.h>
#include "../../src/item.c"
#include "../../src/item_adjustment.c"
#include "../../src/item_adjustment_record.c"
#include "../../src/item_amount.c"
#include "../../src/item_adjustment_list.c"
struct SaveValue21B4 gSaveData;
const struct ItemAdjustmentView gItemAdjustment2[] = {{.category_mask = 0, .items = {[0] = 1, [14] = 16, [29] = 32}}};
const struct ItemAdjustmentView gItemAdjustment3[] = {{.category_mask = 0xffffffffu, .items = {[0] = 1, [14] = 16, [29] = 32}}};
const struct ItemAdjustmentView gItemAdjustment4[] = {{.category_mask = 0x55555555u, .items = {[0] = 1, [14] = 16, [29] = 32}}};
const struct ItemAdjustmentView gItemAdjustment5[] = {{.category_mask = 0xaaaaaaaau, .items = {[0] = 1, [14] = 16, [29] = 32}}};
const struct ItemAdjustmentView gItemAdjustment6[] = {{.category_mask = 0x80000001u, .items = {[0] = 1, [14] = 16, [29] = 32}}};
const struct AmountRecord10 gAmountRecords10[] = {
    {{0}, 0, 0}, {{0}, 90, 0}, {{0}, 250, 0}, {{0}, 101, 0}
};
const struct Item gItems[] = {
    {.property02 = 1000, .half06 = 200, .properties03 = {1, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {2, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {3, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {4, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {5, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {6, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {7, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {8, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {9, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {10, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {11, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {12, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {13, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {14, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {15, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {16, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {17, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {18, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {19, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {20, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {21, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {22, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {23, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {24, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {25, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {26, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {27, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {28, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {29, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {30, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {31, 0, 0, 0, 8}},
    {.property02 = 1000, .half06 = 200, .properties03 = {32, 0, 0, 0, 8}},
};
/* Unrelated item modifier paths must not be reached by this integration test. */
u8 ReadSaveByte(u16 index) { (void)index; assert(0); return 0; }
int main(void)
{
    const struct ItemAdjustmentView *expected_records[] = {
        gItemAdjustment2, gItemAdjustment3, gItemAdjustment4, gItemAdjustment5, gItemAdjustment6
    };
    unsigned selector, item, level;
    for (selector = 0; selector < 256; selector++) {
        const struct ItemAdjustmentView *expected_record =
            selector >= 2 && selector <= 6 ? expected_records[selector - 2] : 0;
        assert(GetItemAdjustmentRecord(selector) == expected_record);
        assert(ReadItemAdjustmentEntry(selector, 0) == 0);
        assert(ItemInAdjustmentList(selector, 0) == 1);
        for (item = 1; item <= 30; item++) {
            assert(ReadItemAdjustmentEntry(selector, item) ==
                (expected_record ? expected_record->items[item - 1] : 0));
        }
        for (item = 1; item <= 40; item++) {
            assert(ItemInAdjustmentList(selector, item) ==
                (expected_record && (item == 1 || item == 16 || item == 32)));
        }
        if (!expected_record) assert(ReadItemAdjustmentEntry(selector, 255) == 0);
        if (!expected_record) assert(ItemAmountAdjustmentApplies(selector, 65535) == 0);
        for (item = 0; item < 32; item++) {
            unsigned applies = expected_record && ((expected_record->category_mask >> item) & 1);
            assert(ItemAmountAdjustmentApplies(selector, item) == applies);
            for (level = 0; level < 4; level++) {
                unsigned reduction = (gAmountRecords10[level].reduction + applies * 10) & 255;
                u32 expected = (u32)(1000 * (100 - (int)reduction) / 100);
                if (expected < 200) expected = 200;
                gSaveData.value = level;
                assert(CalculateItemAmount(selector, item) == expected);
                assert(gSaveData.value == level);
            }
        }
    }
    return 0;
}
