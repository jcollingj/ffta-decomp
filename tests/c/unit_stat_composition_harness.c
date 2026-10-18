#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "item.h"
#include "job.h"
#include "unit.h"
#include "unit_clamped_values.h"
#include "unit_equipment_sums.h"
#include "unit_stat_wrappers.h"

extern u32 GetUnitProperty(const u8 *unit, u8 property);

/* These are real externally linked const tables consumed by item.c and
 * job_property.c. Every modifier is representable by the declared signed
 * char field; omitted table entries are authored zero records. */
const struct Item gItems[303] = {
    [1] = {.modifiers = {-7, -7, -7, -7, -7, -7, -7, -7}},
    [2] = {.modifiers = {-10, -10, -10, -10, -10, -10, -10, -10}},
    [7] = {.modifiers = {-3, 0, 0, 0, 0, 0, 0, 0}},
    [51] = {.modifiers = {120, 2, 3, 4, 30, 30, 5, 0}},
    [148] = {.modifiers = {6, -2, 115, 2, 0, 0, 6, 0}},
    [286] = {.modifiers = {7, 3, 4, 110, 0, 0, 7, 0}},
    [302] = {.modifiers = {5, 120, -1, 1, 0, 0, 5, 0}},
};

const struct Job gJobs[4] = {
    [1] = {.p30 = 17, .p32 = 19, .inherit = 2},
    [2] = {.p30 = 11, .p32 = 13, .inherit = 0},
    [3] = {.p30 = 23, .p32 = 29, .inherit = 255},
};

static u8 sSaveBytes[256];
static u16 sSaveReads[64];
static size_t sSaveReadCount;

u8 ReadSaveByte(u16 index)
{
    assert(index < sizeof(sSaveBytes));
    assert(sSaveReadCount < sizeof(sSaveReads) / sizeof(sSaveReads[0]));
    sSaveReads[sSaveReadCount++] = index;
    return sSaveBytes[index];
}

static void ConfigureSave(u8 enabled, u8 bonus1, u8 bonus2, u8 bonus3, u8 bonus4)
{
    memset(sSaveBytes, 0, sizeof(sSaveBytes));
    sSaveBytes[13] = enabled;
    sSaveBytes[1] = bonus1;
    sSaveBytes[2] = bonus2;
    sSaveBytes[3] = bonus3;
    sSaveBytes[4] = bonus4;
    sSaveReadCount = 0;
}

static void AssertSaveTrace(const u16 *expected, size_t count)
{
    size_t i;
    assert(sSaveReadCount == count);
    for (i = 0; i < count; i++)
        assert(sSaveReads[i] == expected[i]);
}

struct GuardedUnit {
    u32 before;
    struct UnitFieldView unit;
    u32 after;
};

static void InitUnit(struct GuardedUnit *guarded)
{
    memset(guarded, 0, sizeof(*guarded));
    guarded->before = 0x13579BDF;
    guarded->after = 0x2468ACE0;
}

static void AssertGuardedUnitUnchanged(const struct GuardedUnit *before,
                                       const struct GuardedUnit *after)
{
    assert(memcmp(before, after, sizeof(*before)) == 0);
}

static void TestNegativeClampSpecialBonusAndRepeatedItem(void)
{
    static const u16 expectedReads[] = {13, 1, 13, 13, 1};
    struct GuardedUnit actual;
    struct GuardedUnit before;

    InitUnit(&actual);
    actual.unit.half18[4] = 10;
    actual.unit.half18[9] = 51;
    actual.unit.half18[10] = 7;
    actual.unit.half18[11] = 51;
    ConfigureSave(0, 20, 0, 0, 0);
    before = actual;

    /* ID 51: 120 + save[1] 20 = 140. ID 7's signed -3 converts to
     * 0xFFFFFFFD and clamps to 255. Repeated ID 51 contributes 140.
     * The two remaining zero slots are skipped: sum 535; base 10 => 545. */
    assert(GetUnitEquippedStat20(&actual.unit, 1) == 545);
    AssertSaveTrace(expectedReads, sizeof(expectedReads) / sizeof(expectedReads[0]));
    AssertGuardedUnitUnchanged(&before, &actual);

    ConfigureSave(0, 20, 0, 0, 0);
    assert(GetUnitStat20Capped(&actual.unit) == 545);
    AssertSaveTrace(expectedReads, sizeof(expectedReads) / sizeof(expectedReads[0]));
    AssertGuardedUnitUnchanged(&before, &actual);
}

static void TestPositiveSpecialBonusClampsAfterAddition(void)
{
    static const u16 expectedReads[] = {13, 1};
    struct GuardedUnit actual;
    struct GuardedUnit before;

    InitUnit(&actual);
    actual.unit.half18[4] = 10;
    actual.unit.half18[9] = 51;
    ConfigureSave(0, 200, 0, 0, 0);
    before = actual;

    /* Representable signed modifier120 + special bonus200 =320, clamps255;
     * base10 yields265. */
    assert(GetUnitEquippedStat20(&actual.unit, 1) == 265);
    AssertSaveTrace(expectedReads, sizeof(expectedReads) / sizeof(expectedReads[0]));
    AssertGuardedUnitUnchanged(&before, &actual);
}

static void TestFifthSlotOnlyIsVisited(void)
{
    static const u16 expectedReads[] = {13};
    struct GuardedUnit actual;
    struct GuardedUnit before;

    InitUnit(&actual);
    actual.unit.half18[4] = 10;
    actual.unit.half18[13] = 7;
    ConfigureSave(0, 0, 0, 0, 0);
    before = actual;

    /* The sole nonzero item is the fifth item. Its -3 clamps to 255, so the
     * observable result is 265; a four-iteration implementation returns 10. */
    assert(GetUnitEquippedStat20(&actual.unit, 1) == 265);
    AssertSaveTrace(expectedReads, sizeof(expectedReads) / sizeof(expectedReads[0]));
    AssertGuardedUnitUnchanged(&before, &actual);
}

static void TestSignedItemResultsNarrowBeforeStatAddition(void)
{
    static const u16 expectedReads[] = {13, 13};
    struct GuardedUnit actual;
    struct GuardedUnit before;

    InitUnit(&actual);
    actual.unit.half18[5] = 0;
    actual.unit.half18[9] = 1;
    actual.unit.half18[10] = 2;
    ConfigureSave(1, 0, 0, 0, 0);
    before = actual;

    /* The signed results are returned as u32 FFFFFFF9 and FFFFFFF6. Their
     * u32 sum wraps to FFFFFFEF, narrows through u16 to signed -17, then adds
     * to base zero. Negative results are not lower-clamped by this reader. */
    assert(GetUnitEquippedStat22(&actual.unit, 1) == -17);
    AssertSaveTrace(expectedReads, sizeof(expectedReads) / sizeof(expectedReads[0]));
    AssertGuardedUnitUnchanged(&before, &actual);
    ConfigureSave(1, 0, 0, 0, 0);
    assert(GetUnitStat22Capped(&actual.unit) == -17);
    AssertSaveTrace(expectedReads, sizeof(expectedReads) / sizeof(expectedReads[0]));
    AssertGuardedUnitUnchanged(&before, &actual);
}

static void TestAllSelectorContributionsAndDispatch(void)
{
    struct GuardedUnit actual;
    struct GuardedUnit before;
    static const u16 reads10[] = {13, 13, 13};
    static const u16 reads11[] = {13, 3, 13, 13};
    static const u16 reads12[] = {13, 13, 2, 13};
    static const u16 reads13[] = {13, 13, 13, 4};

    InitUnit(&actual);
    actual.unit.half18[4] = 800;
    actual.unit.half18[5] = 800;
    actual.unit.half18[6] = 800;
    actual.unit.half18[7] = 800;
    actual.unit.half18[9] = 302;
    actual.unit.half18[10] = 148;
    actual.unit.half18[11] = 286;
    before = actual;

    /* Each item contributes all of its queried modifier. For selector 10 the
     * total is 5+6+7=18. */
    ConfigureSave(0, 0, 20, 20, 20);
    assert(SelectUnitEquippedStat20Or24(&actual.unit, 1, 1) == 818);
    AssertSaveTrace(reads10, sizeof(reads10) / sizeof(reads10[0]));
    AssertGuardedUnitUnchanged(&before, &actual);

    /* With both mask bits set, bit0/stat20 wins. Stat24 would produce999,
     * making the priority visible in the fixed expected result. */
    ConfigureSave(0, 0, 20, 20, 20);
    assert(SelectUnitEquippedStat20Or24(&actual.unit, 3, 1) == 818);
    AssertSaveTrace(reads10, sizeof(reads10) / sizeof(reads10[0]));
    AssertGuardedUnitUnchanged(&before, &actual);

    /* Selector 11: ID302 120+20=140; ID148 -2 clamps to255; ID286 3.
     * Sum398, base800 gives1198, then the real stat cap gives999. */
    ConfigureSave(0, 0, 20, 20, 20);
    assert(SelectUnitEquippedStat22Or26Capped(&actual.unit, 1) == 999);
    AssertSaveTrace(reads11, sizeof(reads11) / sizeof(reads11[0]));
    AssertGuardedUnitUnchanged(&before, &actual);

    /* Selector 12: ID302 -1 clamps to255; ID148 115+20=135; ID286 4.
     * Sum394, base800 gives1194, then cap999. */
    ConfigureSave(0, 0, 20, 20, 20);
    assert(SelectUnitEquippedStat20Or24Capped(&actual.unit, 2) == 999);
    AssertSaveTrace(reads12, sizeof(reads12) / sizeof(reads12[0]));
    AssertGuardedUnitUnchanged(&before, &actual);

    /* Selector 13: ID302 1; ID148 2; ID286 110+20=130. Sum133 and result933. */
    ConfigureSave(0, 0, 20, 20, 20);
    assert(SelectUnitEquippedStat22Or26(&actual.unit, 2, 1) == 933);
    AssertSaveTrace(reads13, sizeof(reads13) / sizeof(reads13[0]));
    AssertGuardedUnitUnchanged(&before, &actual);

    ConfigureSave(0, 0, 0, 0, 0);
    assert(SelectUnitEquippedStat20Or24(&actual.unit, 0, 1) == 0);
    assert(sSaveReadCount == 0);
    AssertGuardedUnitUnchanged(&before, &actual);
    assert(SelectUnitEquippedStat22Or26(&actual.unit, 0, 1) == 0);
    assert(sSaveReadCount == 0);
    AssertGuardedUnitUnchanged(&before, &actual);
}

static void TestJobInheritanceAndReader16Composition(void)
{
    struct GuardedUnit actual;
    struct GuardedUnit before;

    InitUnit(&actual);
    actual.unit.bytes04[1] = 1; /* unit byte 5: job 1 */
    actual.unit.bytes04[3] = 3; /* unit byte 7: fallback job 3 */
    before = actual;

    /* For selector30, job1 inherits job2, so p30=11. The fallback chain for
     * job3 is job3 (inherit255) -> fallback job1 -> inherited job2: p30=11. */
    assert(GetUnitProperty((const u8 *)&actual.unit, 30) == 11);
    AssertGuardedUnitUnchanged(&before, &actual);
    assert(GetUnitProperty((const u8 *)&actual.unit, 32) == 13);
    AssertGuardedUnitUnchanged(&before, &actual);
    assert(GetJobProperty(3, 1, 30) == 11);
    AssertGuardedUnitUnchanged(&before, &actual);

    actual.unit.unknown34[9] = 1;
    actual.unit.half18[9] = 51;
    actual.unit.half18[10] = 148;
    actual.unit.half18[11] = 286;
    actual.unit.half18[13] = 7;
    ConfigureSave(0, 0, 0, 0, 0);
    before = actual;

    /* Read16 base: inherited p30=11, exact status byte adds1. Property16
     * values are 5+6+7+0=18; fifth slot is traversed. Total30; scale is300. */
    assert(ReadUnitEquipmentValue16(&actual.unit) == 30);
    AssertGuardedUnitUnchanged(&before, &actual);
    assert(ReadUnitEquipmentValue16Scaled10(&actual.unit) == 300);
    AssertGuardedUnitUnchanged(&before, &actual);
    assert(sSaveReadCount == 0); /* Property16 has no save-byte boundary. */
}

static void TestResearchReaders14And15(void)
{
    struct GuardedUnit actual;
    struct GuardedUnit before;

    InitUnit(&actual);
    actual.unit.signedD0[1] = 20;
    actual.unit.bytes04[1] = 1;
    actual.unit.bytes04[3] = 3;
    actual.unit.half18[9] = 1;
    actual.unit.half18[10] = 2;
    ConfigureSave(0, 0, 0, 0, 0);
    before = actual;

    /* Research only: signed modifier values -7 and -10 return as u32 and
     * sum to FFFFFFEF. Reader14 adds unsigned base20 before signed-short cast:
     * 20+FFFFFFEF wraps to3, then lower clamp leaves3. Reader15's inherited
     * base13 plus that sum narrows to signed -4, then clamps to0. */
    assert(ReadUnitEquipmentValue14(&actual.unit) == 3);
    AssertGuardedUnitUnchanged(&before, &actual);
    assert(ReadUnitEquipmentValue15(&actual.unit) == 0);
    AssertGuardedUnitUnchanged(&before, &actual);
    assert(sSaveReadCount == 0);

    /* Positive research vector: five repeated ID51 values 30 produce150.
     * Reader14 base40 gives190. Reader15 base13 gives163, then upper-clamps100. */
    actual.unit.signedD0[1] = 40;
    actual.unit.half18[9] = 51;
    actual.unit.half18[10] = 51;
    actual.unit.half18[11] = 51;
    actual.unit.half18[12] = 51;
    actual.unit.half18[13] = 51;
    before = actual;
    assert(ReadUnitEquipmentValue14(&actual.unit) == 190);
    AssertGuardedUnitUnchanged(&before, &actual);
    assert(ReadUnitEquipmentValue15(&actual.unit) == 100);
    AssertGuardedUnitUnchanged(&before, &actual);
    assert(sSaveReadCount == 0);
}

int main(void)
{
    TestNegativeClampSpecialBonusAndRepeatedItem();
    TestPositiveSpecialBonusClampsAfterAddition();
    TestFifthSlotOnlyIsVisited();
    TestSignedItemResultsNarrowBeforeStatAddition();
    TestAllSelectorContributionsAndDispatch();
    TestJobInheritanceAndReader16Composition();
    TestResearchReaders14And15();
    return 0;
}
