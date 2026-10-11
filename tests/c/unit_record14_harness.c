#include <assert.h>
#include <string.h>
#include "../../src/unit_record14.c"
const struct UnitRecord14 gUnitRecords14[] = {
    {{0}, 0, 65535, 0, 0, 15, 0, {0, 1, 255}},
    {{0}, 0x1234, 0xabcd, 2, 1, 13, 255, {1, 255, 0}},
    {{0}, 0x5678, 0x9abc, 6, 254, 11, 2, {255, 0, 1}},
};
u8 gRecordCopyBuffer[512];
static u8 source[32];
static unsigned special, property_calls, source_calls, copy_calls, source_index;
static unsigned expected_property, expected_slot;
u8 UnitField01AboveOne(const struct UnitFieldView *unit)
{
    assert(unit->bytes04[0] < 3);
    return special;
}
u32 GetUnitProperty(const u8 *unit, u8 property)
{
    (void)unit;
    assert(property == expected_property);
    property_calls++;
    return 0x12345;
}
const void *ReadRecordCopySource(u8 index)
{
    unsigned i;
    source_calls++;
    source_index = index;
    for (i = 0; i < 32; i++) source[i] = index + i;
    return source;
}
static void copy_record(void *dest, const void *src, u32 size)
{
    assert(dest == gRecordCopyBuffer + expected_slot * 32);
    assert(src == source && size == 32);
    copy_calls++;
    memcpy(dest, src, size);
}
void (*const gCopyRecordMemory)(void *, const void *, u32) = copy_record;
int main(void)
{
    struct UnitFieldView unit;
    u8 expected[512];
    unsigned record, selector, i;
    memset(&unit, 0, sizeof(unit));
    for (special = 0; special < 2; special++) {
        for (record = 0; record < 3; record++) {
            const struct UnitRecord14 *entry = &gUnitRecords14[record];
            unit.bytes04[0] = record;
            for (selector = 0; selector < 256; selector++) {
                unsigned id = selector == 1 ? entry->source_a : entry->source_b;
                expected_slot = selector == 1 ? entry->slot_a : entry->slot_b;
                expected_property = selector == 1 ? 6 : 7;
                memset(gRecordCopyBuffer, 0xa5, sizeof(gRecordCopyBuffer));
                memcpy(expected, gRecordCopyBuffer, sizeof(expected));
                property_calls = source_calls = copy_calls = 0;
                if (special && id) {
                    for (i = 0; i < 32; i++) expected[expected_slot * 32 + i] = id - 1 + i;
                }
                assert(LoadUnitRecordSlot(&unit, selector) == (special ? expected_slot : 0x45));
                assert(property_calls == !special);
                assert(source_calls == (special && id) && copy_calls == source_calls);
                if (special && id) assert(source_index == id - 1);
                assert(memcmp(expected, gRecordCopyBuffer, sizeof(expected)) == 0);
                property_calls = 0;
                expected_property = selector == 1 ? 10 : 11;
                assert(ReadUnitRecordProperty10Or11(&unit, selector) ==
                    (special ? entry->properties09[selector == 1 ? 1 : 2] : 0x45));
                assert(property_calls == !special);
            }
            property_calls = 0;
            expected_property = 4;
            assert(ReadUnitRecordProperty04(&unit) == (special ? entry->property04 : 0x2345));
            expected_property = 5;
            assert(ReadUnitRecordProperty05(&unit) == (special ? entry->property05 : 0x2345));
            assert(property_calls == (special ? 0 : 2));
            property_calls = 0;
            expected_property = 9;
            assert(ReadUnitRecordProperty09(&unit) == (special ? entry->properties09[0] : 0x45));
            assert(property_calls == !special);
        }
    }
    return 0;
}
