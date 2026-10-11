#include <assert.h>
#include <string.h>
#include "../../src/item.c"
#include "../../src/ability_state.c"
#include "../../src/equipment_inspect.c"
const struct Item gItems[3] = {[1]={.ability_set=1}};
const struct ItemAbilitySet gItemAbilitySets[2] = {
    {0}, {7,0,{{7,1},{7,2},{7,3},{0,1},{8,2},{7,4},{7,3}}}
};
static const struct AbilityListEntry list0[5] = {
    {0}, {.requirement=1}, {.requirement=64}, {.requirement=127}, {.requirement=255}
};
static const struct AbilityListEntry list1[5] = {
    {0}, {.requirement=255}, {.requirement=127}, {.requirement=64}, {.requirement=1}
};
const struct AbilityListEntry *const gAbilityLists[] = {list0, list1};
u8 ReadSaveByte(u16 index) { (void)index; assert(0); return 0; }
static union { u32 align; u8 bytes[0x108]; } storage;
static u8 before[0x108], removed[0x108];
static unsigned selected_slot, new_item, provided, mode, expected_flags;
static unsigned copy_calls, provider_calls, check_calls, check_result;
static void CopyMemory(void *dest, const void *src, u32 size)
{
    assert(src == storage.bytes + 0x2a && size == 10);
    assert(memcmp(storage.bytes, before, sizeof(before)) == 0);
    copy_calls++;
    memcpy(dest, src, size);
}
void (*const gCopyRecordMemory)(void *, const void *, u32) = CopyMemory;
u8 EquipmentProvidesAbility(const u8 *unit, u8 ability)
{
    assert(unit == storage.bytes);
    assert(ability >= 1 && ability <= 4);
    assert(memcmp(unit, removed, sizeof(removed)) == 0);
    provider_calls++;
    return (provided >> (ability-1)) & 1;
}
u8 CheckEquipmentSet(struct EquipmentView *unit, const u16 *items, signed char slot, u8 flags)
{
    unsigned i;
    assert((u8 *)unit == storage.bytes);
    assert(memcmp(storage.bytes, before, sizeof(before)) == 0);
    for (i=0; i<5; i++)
        assert(items[i] == (i == selected_slot ? new_item : unit->items[i]));
    assert(slot == (mode ? (signed char)selected_slot : -1));
    assert(flags == expected_flags);
    check_calls++;
    return check_result;
}
int main(void)
{
    unsigned progress, layout, list, old_item, ability, expected_calls;
    struct EquipmentView *unit=(struct EquipmentView *)storage.bytes;
    for (selected_slot=0; selected_slot<5; selected_slot++)
    for (progress=0; progress<256; progress++)
    for (provided=0; provided<16; provided++)
    for (layout=0; layout<2; layout++) {
        memset(storage.bytes, 0xa5, sizeof(storage.bytes));
        list=progress & 1;
        storage.bytes[6]=list;
        storage.bytes[7]=7;
        storage.bytes[0x3a]=layout ? 3 : 1;
        storage.bytes[0x3b]=layout ? 3 : 2;
        storage.bytes[0x3c]=3;
        for (ability=1; ability<=4; ability++) storage.bytes[0x40+ability]=progress;
        old_item=(progress % 3) != 0;
        unit->items[selected_slot]=old_item;
        new_item=(progress / 3) % 3;
        mode=(progress & 2) ? 255 : 0;
        check_result=(progress & 4) ? 0x81 : 0x10;
        memcpy(before, storage.bytes, sizeof(before));
        memcpy(removed, before, sizeof(removed));
        removed[0x2a+selected_slot*2]=removed[0x2b+selected_slot*2]=0;
        expected_flags=expected_calls=0;
        if (old_item && old_item != new_item) {
            for (ability=1; ability<=4; ability++) {
                if ((progress & 127) < gAbilityLists[list][ability].requirement) {
                    expected_calls += ability == 3 ? 2 : 1;
                    if (!(provided & (1u << (ability-1)))) {
                        if (ability==3) expected_flags |= 8;
                        else if (!layout && ability==2) expected_flags |= 4;
                        else if (!layout && ability==1) expected_flags |= 2;
                    }
                }
            }
        }
        copy_calls=provider_calls=check_calls=0;
        assert(InspectEquipment(unit,new_item,selected_slot,mode) == (expected_flags | check_result));
        assert(copy_calls==1 && check_calls==1 && provider_calls==expected_calls);
        assert(memcmp(storage.bytes,before,sizeof(before))==0);
    }
    return 0;
}
