#include <assert.h>
#include <string.h>
#include "../../include/equipment_scan.h"
#include "../../include/ability_state.h"
#include "../../include/item_abilities.h"
#include "../../src/nonmatching/equipment_replace_direct.c"

const struct ItemAbilitySet gItemAbilitySets[] = {
    {3, 0, {{7, 4}, {7, 5}, {255, 0}}},
};
static struct AbilityListEntry ability_entries[256];
static u8 ability_provider[256];
static unsigned sync_count, refresh_count, provider_count;
static struct EquipmentView *expected_unit;
static u16 expected_item;
static u8 expected_slot;

u32 GetItemProperty(u16 item, u8 property)
{
    if (property == 18) return 0;
    if (property == 3) return item == 1 ? 27 : 0;
    assert(0);
    return 0;
}

const struct AbilityListEntry *GetAbilityListEntry(u8 list, u8 ability)
{
    assert(list == 6);
    return &ability_entries[ability];
}

u8 EquipmentProvidesAbility(const u8 *unit, u8 ability)
{
    assert(unit == (const u8 *)expected_unit);
    assert(expected_unit->items[expected_slot] == 0);
    provider_count++;
    return ability_provider[ability];
}

void SyncPairedAbilityState(u8 *unit, u8 ability)
{
    assert(unit == (u8 *)expected_unit);
    assert(ability == 4);
    sync_count++;
}

void ApplyItemAbilityFlags(u8 *unit, u16 item)
{
    assert(unit == (u8 *)expected_unit && item == expected_item);
    assert(expected_unit->items[expected_slot] == item);
    refresh_count++;
}

int main(void)
{
    union { u32 alignment; u8 bytes[0x140]; } storage;
    struct EquipmentView *unit = (struct EquipmentView *)storage.bytes;
    u8 *bytes = storage.bytes;

    memset(storage.bytes, 0, sizeof(storage.bytes));
    bytes[6] = 6;
    bytes[7] = 7;
    unit->items[2] = 1;
    bytes[0x3a] = 4;
    bytes[0x3b] = 5;
    bytes[0x3c] = 9;
    bytes[0x3d] = 0x55;
    ability_entries[4].requirement = 2;
    ability_entries[5].requirement = 2;
    ((struct UnitAbilityStateView *)unit)->states[4] = 0x81;
    ((struct UnitAbilityStateView *)unit)->states[5] = 0x81;
    ability_provider[4] = 0;
    ability_provider[5] = 1;
    expected_unit = unit;
    expected_item = 2;
    expected_slot = 2;
    ReplaceEquipmentDirect(unit, 2, 2);
    assert(unit->items[2] == 2);
    assert(bytes[0x3a] == 0 && bytes[0x3b] == 5 && bytes[0x3c] == 9);
    assert(bytes[0x3d] == 0);
    assert(((struct UnitAbilityStateView *)unit)->states[4] == 1);
    assert(((struct UnitAbilityStateView *)unit)->states[5] == 0x81);
    assert(provider_count == 2 && sync_count == 1 && refresh_count == 1);

    /* A 255 job byte matching the unit job follows the normal branch. */
    bytes[7] = 255;
    unit->items[2] = 1;
    bytes[0x3d] = 0x55;
    expected_item = 2;
    expected_slot = 2;
    ReplaceEquipmentDirect(unit, 2, 2);
    assert(bytes[0x3d] == 0x55);
    assert(unit->items[2] == 2 && refresh_count == 2);
    bytes[7] = 7;

    /* The same nonzero item is a full no-op. */
    {
        u8 before[sizeof(storage.bytes)];
        memcpy(before, storage.bytes, sizeof(before));
        expected_item = 2;
        expected_slot = 2;
        provider_count = sync_count = 0;
        ReplaceEquipmentDirect(unit, 2, 2);
        assert(memcmp(before, storage.bytes, sizeof(before)) == 0);
        assert(provider_count == 0 && sync_count == 0 && refresh_count == 2);
    }

    /* Replacing an empty slot only writes the item and refreshes once. */
    unit->items[1] = 0;
    expected_item = 3;
    expected_slot = 1;
    ReplaceEquipmentDirect(unit, 3, 1);
    assert(unit->items[1] == 3 && refresh_count == 3);
    return 0;
}
