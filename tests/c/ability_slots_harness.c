/* Portable state behavior only; ARM layout/bytes are checked by make compare. */
#include <assert.h>
#include <string.h>
#include "../../src/ability_slots.c"

/* Isolate the routines under test from the selector dispatcher. */
u32 ReadUnitField(const struct UnitFieldView *unit, u8 selector)
{
    assert(selector == 3);
    return ((const u8 *)unit)[6];
}

static void test_paired_state(void)
{
    u8 unit[0x108], expected[0x108];
    unsigned category, ability, value;
    /* Exercise both directions, every byte value, and non-pair no-ops.
     * Comparing the whole unit catches writes outside the target byte. */
    for (category = 0; category < 256; category++) {
        for (ability = 0; ability < 256; ability++) {
            memset(unit, 0x5a, sizeof(unit));
            unit[6] = category;
            unit[0x40 + 26] = 0x87;
            unit[0x40 + 33] = 0x12;
            memcpy(expected, unit, sizeof(unit));
            if (category == 4 && ability == 26) expected[0x40 + 33] = 0x87;
            if (category == 4 && ability == 33) expected[0x40 + 26] = 0x12;
            SyncPairedAbilityState(unit, ability);
            assert(memcmp(unit, expected, sizeof(unit)) == 0);
        }
    }
    for (value = 0; value < 256; value++) {
        memset(unit, 0, sizeof(unit));
        unit[6] = 4;
        unit[0x40 + 26] = value;
        SyncPairedAbilityState(unit, 26);
        assert(unit[0x40 + 33] == value);
        unit[0x40 + 26] = 0;
        SyncPairedAbilityState(unit, 33);
        assert(unit[0x40 + 26] == value);
    }
}

static void test_slot_clearing(void)
{
    u8 unit[0x108], expected[0x108];
    unsigned mask, slot;
    /* Every combination of duplicates must clear only the first match. */
    for (mask = 0; mask < 8; mask++) {
        memset(unit, 0x5a, sizeof(unit));
        for (slot = 0; slot < 3; slot++)
            unit[0x3a + slot] = (mask & (1u << slot)) ? 9 : 7;
        memcpy(expected, unit, sizeof(unit));
        for (slot = 0; slot < 3; slot++) {
            if (mask & (1u << slot)) {
                expected[0x3a + slot] = 0;
                break;
            }
        }
        ClearMatchingAbilitySlot(unit, 9);
        assert(memcmp(unit, expected, sizeof(unit)) == 0);
    }
    memset(unit, 0x5a, sizeof(unit));
    unit[0x3a] = 0;
    memcpy(expected, unit, sizeof(unit));
    ClearMatchingAbilitySlot(unit, 0);
    assert(memcmp(unit, expected, sizeof(unit)) == 0);
    /* Null plus zero writes through null in the original: not a valid input. */
    ClearMatchingAbilitySlot(0, 9);
}

int main(void)
{
    test_paired_state();
    test_slot_clearing();
    return 0;
}
