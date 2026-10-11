#include <assert.h>
#include <string.h>
#include "../../src/unit_fields.c"
#include "../../src/item.c"
#include "../../src/ability_state.c"
#include "../../src/nonmatching/equipment_provider.c"
#include "../../src/equipment_inspect.c"
const struct Item gItems[10] = {
    [1]={.ability_set=1},[2]={.ability_set=2},[3]={.ability_set=3},
    [4]={.ability_set=4},[5]={.ability_set=5},[6]={.ability_set=6},
    [7]={.ability_set=7},[8]={.ability_set=8},[9]={.ability_set=9}
};
const struct ItemAbilitySet gItemAbilitySets[10] = {
    {0}, {1,0,{{7,26}}}, {1,0,{{7,33}}}, {1,0,{{7,9}}},
    {2,0,{{0,26},{8,33}}}, {1,0,{{255,9}}}, {0},
    {9,0,{{8,26},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{7,33}}},
    {2,0,{{7,26},{7,33}}}, {1,0,{{7,0}}}
};
static const struct AbilityListEntry abilities[34] = {
    [9]={.requirement=100},[26]={.requirement=100},[33]={.requirement=100}
};
const struct AbilityListEntry *const gAbilityLists[5] = {abilities,abilities,abilities,abilities,abilities};
u8 ReadSaveByte(u16 index) { (void)index; assert(0); return 0; }
static void CopyMemory(void *dest, const void *src, u32 size) { assert(size==10); memcpy(dest,src,size); }
void (*const gCopyRecordMemory)(void *, const void *, u32) = CopyMemory;
static unsigned expected_flags, expected_slot, checks;
static u8 before[0x108];
u8 CheckEquipmentSet(struct EquipmentView *unit, const u16 *items, signed char slot, u8 flags)
{
    unsigned i;
    assert(memcmp(unit,before,sizeof(before))==0);
    assert(slot==-1 && flags==expected_flags);
    for (i=0;i<5;i++) assert(items[i]==(i==expected_slot ? 0 : unit->items[i]));
    checks++;
    return 0x80;
}
static unsigned Expected(unsigned item, unsigned job, unsigned field, unsigned ability)
{
    const struct ItemAbilitySet *set=&gItemAbilitySets[item];
    unsigned j;
    if (!item) return 0;
    for (j=0;j<set->count;j++) {
        unsigned given=set->entries[j][1];
        if (!set->entries[j][0] || set->entries[j][0]!=job) continue;
        if (given==ability) return 1;
        if (field==4 && ((given==26 && ability==33) || (given==33 && ability==26))) return 1;
    }
    return 0;
}
int main(void)
{
    union {u32 align; u8 bytes[0x108];} storage;
    struct EquipmentView *unit=(struct EquipmentView *)storage.bytes;
    static const unsigned jobs[]={0,7,8,255}, fields[]={0,4,255};
    unsigned slot,item,job,field,ability,other;
    for (slot=0;slot<5;slot++) for (item=0;item<10;item++)
    for (job=0;job<4;job++) for (field=0;field<3;field++) {
        memset(&storage,0,sizeof(storage));
        storage.bytes[6]=fields[field]; storage.bytes[7]=jobs[job];
        unit->items[slot]=item;
        memcpy(before,storage.bytes,sizeof(before));
        for (ability=0;ability<256;ability++)
            assert(EquipmentProvidesAbility(storage.bytes,ability)==Expected(item,jobs[job],fields[field],ability));
        assert(memcmp(before,storage.bytes,sizeof(before))==0);
    }
    for (slot=0;slot<5;slot++) for (other=0;other<5;other++) for (field=0;field<2;field++) {
        memset(&storage,0,sizeof(storage));
        storage.bytes[6]=field ? 4 : 0; storage.bytes[7]=7;
        storage.bytes[0x3c]=26;
        unit->items[other]=2;
        unit->items[slot]=1;
        expected_slot=slot;
        expected_flags=field && other!=slot ? 0 : 8;
        checks=0;
        memcpy(before,storage.bytes,sizeof(before));
        assert(InspectEquipment(unit,0,slot,0)==(expected_flags|0x80));
        assert(checks==1 && memcmp(before,storage.bytes,sizeof(before))==0);
    }
    return 0;
}
