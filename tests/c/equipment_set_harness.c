#include <assert.h>
#include <string.h>
#include "../../src/nonmatching/equipment_set.c"

const u32 gEquipmentClassMasks[1] = {0xffffffffu & ~(1u << 19)};
static u8 category[400], kind[400], bit7[400];
static u8 support_value;
static u8 job_value = 2;
static void Clear(void *p, u32 n) { memset(p, 0, n); }
static void Copy(void *d, const void *s, u32 n) { memcpy(d, s, n); }
void (*const gClearMemory)(void *, u32) = Clear;
void (*const gCopyRecordMemory)(void *, const void *, u32) = Copy;
u8 ReadAbilitySlotBValue(const u8 *unit) { (void)unit; return support_value; }
u32 GetUnitProperty(const u8 *unit, u8 property) { (void)unit; assert(property==36); return 0; }
u32 GetItemProperty(u16 item, u8 property) {
    assert(item < 400);
    if (property==3) return category[item];
    if (property==6) return kind[item];
    if (property==7) return bit7[item];
    assert(0); return 0;
}
u32 ReadUnitField(const struct UnitFieldView *unit, u8 field) { (void)unit; assert(field==4); return job_value; }

static u8 run(u8 support, u8 flags, u8 selected, u16 a,u16 b,u16 c,u16 d,u16 e) {
    struct EquipmentView u, before; u16 p[5]={a,b,c,d,e}, original[5];
    u8 result;
    memset(&u,0,sizeof(u)); support_value=support;
    before=u; memcpy(original,p,sizeof(p));
    result=CheckEquipmentSet(&u,p,selected,flags);
    assert(memcmp(&before,&u,sizeof(u))==0 && memcmp(original,p,sizeof(p))==0);
    return result;
}
int main(void) {
    memset(category,0,sizeof(category)); memset(kind,0,sizeof(kind)); memset(bit7,0,sizeof(bit7));
    /* Category 22 is restricted to unit byte 6 == 4, independent of the mask. */
    category[1]=22; kind[1]=7;
    { struct EquipmentView u; u16 p[5]={1,0,0,0,0}; memset(&u,0,sizeof(u));
      assert(CheckEquipmentSet(&u,p,0,0)==0x81); ((u8*)&u)[6]=4; assert(CheckEquipmentSet(&u,p,0,0)==0); }
    /* Category 20 can pass a missing mask bit only with support value 15. */
    category[2]=20; kind[2]=7;
    { struct EquipmentView u; u16 p[5]={2,0,0,0,0}; memset(&u,0,sizeof(u));
      assert(CheckEquipmentSet(&u,p,1,0)==1); support_value=15;
      assert(CheckEquipmentSet(&u,p,0,0)==0); assert(CheckEquipmentSet(&u,p,0,4)==0x81); }
    /* One item of kind 3 is allowed; a duplicate fails at the second slot. */
    category[10]=1; kind[10]=3; category[11]=1; kind[11]=3;
    assert(run(0,0,0,10,0,0,0,0)==0);
    assert(run(0,0,1,10,11,0,0,0)==0x81);
    assert(run(0,0,0,10,11,0,0,0)==1);
    /* Kind 7 is uncapped; unsupported kind values are ignored. */
    kind[12]=7; kind[13]=99; category[12]=1; category[13]=1;
    assert(run(0,0,0,12,12,12,12,12)==0);
    assert(run(0,0,0,13,13,13,13,13)==0);
    /* kind 1 paired with kind 2 follows support 6 and the job counter high bit. */
    category[260]=1; kind[260]=1; category[21]=1; kind[21]=2;
    assert(run(6,0,0,260,21,0,0,0)==0);
    assert(run(0,0,0,260,21,0,0,0)==1);
    assert(run(6,4,0,260,21,0,0,0)==1); /* flag 4 disables support */
    /* support 7 allows kind 1 with property-7 bit 1 by consuming kind-2 count. */
    category[22]=1; kind[22]=1; bit7[22]=2;
    assert(run(7,0,0,22,0,0,0,0)==0);
    /* Support 8 permits two ordinary kind-1 items, but a third is invalid. */
    category[30]=1; kind[30]=1; category[31]=1; kind[31]=1; category[32]=1; kind[32]=1;
    assert(run(8,0,0,30,31,0,0,0)==0);
    assert(run(8,0,2,30,31,32,0,0)==0x81);
    assert(run(7,0,0,30,22,0,0,0)==0);
    assert(run(0,0,0,30,22,0,0,0)==1);
    assert(run(7,4,0,30,22,0,0,0)==1);
    /* Check the full job-byte exception list for special kind-1 IDs. */
    { unsigned job;
      for (job=0;job<256;job++) {
        unsigned allowed = job==2 || job==3 || job==13 || job==27 || job==28 || job==37 ||
                           job==83 || (job>=112 && job<=115);
        job_value=job;
        assert(run(0,0,0,260,0,0,0,0)==(allowed ? 0 : 0x81));
        assert(run(15,0,0,260,0,0,0,0)==0);
      }
      job_value=2;
    }
    /* Item-specific categories 23/25 allow three IDs only for unit byte 6 == 4. */
    category[311]=25; kind[311]=7; category[316]=23; kind[316]=7;
    category[287]=25; kind[287]=7;
    assert(run(0,0,1,311,0,0,0,0)==1);
    assert(run(0,0,1,316,0,0,0,0)==1);
    assert(run(0,0,1,287,0,0,0,0)==1);
    { struct EquipmentView u; u16 p[5]={311,316,287,0,0}; memset(&u,0,sizeof(u));
      ((u8*)&u)[6]=4; assert(CheckEquipmentSet(&u,p,0,0)==0); }
    /* A selected invalid slot returns 0x81 only when the signed byte equals its index. */
    category[40]=20; kind[40]=7;
    assert(run(0,0,0,40,0,0,0,0)==0x81);
    assert(run(0,0,1,40,0,0,0,0)==1);
    assert(run(0,0,128,40,0,0,0,0)==1);
    assert(run(0,0,255,40,0,0,0,0)==1);
    return 0;
}
