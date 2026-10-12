#include <assert.h>
#include <string.h>
#include "../../src/unit_fields.c"
#include "../../src/item.c"
#include "../../src/ability_state.c"
#include "../../src/item_abilities.c"
#include "../../src/nonmatching/equipment_provider.c"
#include "../../src/nonmatching/equipment_replace.c"

const struct Item gItems[5] = {
    [1] = {.ability_set = 1, .properties03 = {27}},
    [2] = {.ability_set = 2},
    [3] = {.ability_set = 3},
    [4] = {.ability_set = 4}
};
const struct ItemAbilitySet gItemAbilitySets[5] = {
    {0},
    {2, 0, {{7, 10}, {255, 12}}},
    {1, 0, {{7, 13}}},
    {1, 0, {{7, 10}}},
    {1, 0, {{7, 26}}}
};
static const struct AbilityListEntry abilityList[34] = {
    [10] = {.requirement = 9}, [12] = {.requirement = 9},
    [13] = {.requirement = 9}, [26] = {.requirement = 9},
    [33] = {.requirement = 9}
};
const struct AbilityListEntry *const gAbilityLists[5] = {
    abilityList, abilityList, abilityList, abilityList, abilityList
};

struct AdjustCall { u16 item; u8 mode; };
static struct AdjustCall calls[16];
static unsigned callCount;
static u8 gameFlag;

u8 ReadSaveByte(u16 index) { (void)index; assert(0); return 0; }
u8 GetGameFlag(u16 flag) { assert(flag == 62); return gameFlag; }
void AdjustInventoryEntryByte3(u16 item, u8 mode)
{
    assert(callCount < sizeof(calls) / sizeof(calls[0]));
    calls[callCount].item = item;
    calls[callCount].mode = mode;
    callCount++;
}
static struct EquipmentView *Unit(u8 *storage)
{
    return (struct EquipmentView *)storage;
}

static void Reset(u8 *storage)
{
    memset(storage, 0, 0x108);
    callCount = 0;
    gameFlag = 0;
}

static void TestEmptyAndSameItem(void)
{
    union { u32 align; u8 bytes[0x108]; } storage;
    struct EquipmentView *unit = Unit(storage.bytes);
    u8 before[0x108];

    Reset(storage.bytes);
    storage.bytes[7] = 7;
    ReplaceEquipment(unit, 2, 1);
    assert(unit->items[1] == 2);
    assert(callCount == 1 && calls[0].item == 2 && calls[0].mode == 1);
    assert(storage.bytes[0x40 + 13] == 0x80);

    Reset(storage.bytes);
    unit->items[2] = 1;
    storage.bytes[0x40 + 10] = 0x83;
    storage.bytes[0x3c] = 10;
    memcpy(before, storage.bytes, sizeof(before));
    ReplaceEquipment(unit, 1, 2);
    assert(callCount == 0);
    assert(memcmp(before, storage.bytes, sizeof(before)) == 0);
}

static void TestRemovalAndAlternateProvider(void)
{
    union { u32 align; u8 bytes[0x108]; } storage;
    struct EquipmentView *unit = Unit(storage.bytes);

    Reset(storage.bytes);
    unit->items[0] = 1;
    storage.bytes[7] = 7;
    storage.bytes[6] = 0;
    storage.bytes[0x40 + 10] = 0x83;
    storage.bytes[0x40 + 12] = 0x81;
    storage.bytes[0x3c] = 10;
    storage.bytes[0x3b] = 12;
    storage.bytes[0x3d] = 99;
    ReplaceEquipment(unit, 0, 0);
    assert(unit->items[0] == 0);
    assert(storage.bytes[0x40 + 10] == 3);
    assert(storage.bytes[0x40 + 12] == 0x81);
    assert(storage.bytes[0x3c] == 0 && storage.bytes[0x3b] == 12);
    assert(storage.bytes[0x3d] == 0);
    assert(callCount == 1 && calls[0].item == 1 && calls[0].mode == 0);

    Reset(storage.bytes);
    unit->items[0] = 1;
    unit->items[1] = 3;
    storage.bytes[7] = 7;
    storage.bytes[0x40 + 10] = 2;
    storage.bytes[0x3c] = 10;
    ReplaceEquipment(unit, 0, 0);
    assert(storage.bytes[0x40 + 10] == 2);
    assert(storage.bytes[0x3c] == 10);
    assert(callCount == 1 && calls[0].item == 1 && calls[0].mode == 0);
}

static void TestInventoryGatesAndPairedState(void)
{
    unsigned gate;
    for (gate = 0; gate < 3; gate++) {
        union { u32 align; u8 bytes[0x108]; } storage;
        struct EquipmentView *unit = Unit(storage.bytes);
        Reset(storage.bytes);
        unit->items[0] = 1;
        if (gate == 0) ((struct UnitFieldView *)unit)->half18[8] = 0x8000;
        if (gate == 1) ((struct UnitFieldView *)unit)->half18[8] = 0x2000;
        if (gate == 2) gameFlag = 1;
        ReplaceEquipment(unit, 2, 0);
        assert(unit->items[0] == 2);
        assert(callCount == 0);
    }

    {
        union { u32 align; u8 bytes[0x108]; } storage;
        struct EquipmentView *unit = Unit(storage.bytes);
        Reset(storage.bytes);
        unit->items[0] = 4;
        storage.bytes[6] = 4;
        storage.bytes[7] = 7;
        storage.bytes[0x40 + 26] = 0x82;
        storage.bytes[0x40 + 33] = 0x82;
        storage.bytes[0x3a] = 26;
        ReplaceEquipment(unit, 0, 0);
        assert(storage.bytes[0x40 + 26] == 2);
        assert(storage.bytes[0x40 + 33] == 2);
        assert(storage.bytes[0x3a] == 0);
    }
}

int main(void)
{
    TestEmptyAndSameItem();
    TestRemovalAndAlternateProvider();
    TestInventoryGatesAndPairedState();
    {
        union { u32 align; u8 bytes[0x108]; } storage;
        struct EquipmentView *unit = Unit(storage.bytes);
        Reset(storage.bytes);
        unit->items[0] = 1;
        storage.bytes[7] = 255;
        storage.bytes[0x3d] = 99;
        storage.bytes[0x3c] = 12;
        storage.bytes[0x40 + 12] = 0x81;
        ReplaceEquipment(unit, 0, 0);
        assert(storage.bytes[0x3d] == 99);
        assert(storage.bytes[0x3c] == 0 && storage.bytes[0x40 + 12] == 1);
    }
    return 0;
}
