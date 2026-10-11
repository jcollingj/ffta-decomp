#include <assert.h>
#include <string.h>
#include "../../src/equipment_scan.c"
static u32 masks[6];
static unsigned calls[32], call_count, replacements;
u32 InspectEquipment(struct EquipmentView *unit, u16 item, u8 slot, u8 mode)
{
    assert(slot < 5 && item == unit->items[slot] && item < 6 && mode == 1);
    assert(call_count < 32);
    calls[call_count++] = slot;
    return masks[item];
}
void ReplaceEquipment(struct EquipmentView *unit, u16 item, u8 slot)
{
    assert(item == 0 && slot < 5);
    replacements++;
    unit->items[slot] = 0;
}
int main(void)
{
    struct EquipmentView unit, expected;
    static const u8 modes[] = {0, 1, 255};
    unsigned slot, mask, mode, i;
    for (slot = 0; slot < 5; slot++) {
        for (mask = 0; mask < 256; mask++) {
            for (mode = 0; mode < sizeof(modes); mode++) {
                unsigned match = (mask & 0x81) == 0x81;
                memset(&unit, 0xa5, sizeof(unit));
                memset(unit.items, 0, sizeof(unit.items));
                unit.items[slot] = 1;
                masks[1] = mask | 0x10000;
                expected = unit;
                if (match && modes[mode]) expected.items[slot] = 0;
                call_count = replacements = 0;
                assert(ScanEquipmentMask81(&unit, modes[mode]) == match);
                assert(call_count == 1 && calls[0] == slot);
                assert(replacements == (match && modes[mode]));
                assert(memcmp(&unit, &expected, sizeof(unit)) == 0);
            }
        }
    }
    memset(&unit, 0, sizeof(unit));
    for (i = 0; i < 5; i++) { unit.items[i] = i + 1; masks[i + 1] = 0; }
    masks[4] = 0x81;
    expected = unit;
    expected.items[3] = 0;
    call_count = replacements = 0;
    assert(ScanEquipmentMask81(&unit, 1) == 1);
    {
        static const unsigned trace[] = {0, 1, 2, 3, 1, 2, 4};
        assert(call_count == sizeof(trace) / sizeof(trace[0]));
        assert(memcmp(calls, trace, sizeof(trace)) == 0);
    }
    assert(replacements == 1 && memcmp(&unit, &expected, sizeof(unit)) == 0);
    memset(&unit, 0, sizeof(unit));
    call_count = replacements = 0;
    assert(ScanEquipmentMask81(&unit, 255) == 0);
    assert(call_count == 0 && replacements == 0);
    return 0;
}
