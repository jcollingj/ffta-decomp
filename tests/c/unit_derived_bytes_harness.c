#include <assert.h>
#include <string.h>
#include "job.h"
#include "unit_derived_bytes.h"
#include "unit_equipment_sums.h"
#include "unit_status.h"

#define RECORD_SIZE 0x108

struct GuardedRecord {
    u8 before[8];
    struct UnitRecord record;
    u8 after[8];
};

static struct UnitRecord *sRecord;
static u32 sProperties[3];
static unsigned sPropertyCalls;
static u32 sEquipmentSum;
static unsigned sEquipmentCalls;
static u8 sFlag;
static u8 sModeAfterFlag;
static int sMutateMode;
static unsigned sFlagCalls;
static u8 sExpectedModeAtFlag;
#ifdef REAL_COMPOSITION
static u8 sExpectedJob;
static u8 sExpectedFallback;
#endif

#ifndef REAL_COMPOSITION
u32 GetUnitProperty(const u8 *unit, u8 property)
{
    static const u8 selectors[3] = {34, 35, 31};
    assert(unit == (const u8 *)sRecord);
    assert(sPropertyCalls < 3);
    assert(property == selectors[sPropertyCalls]);
    sPropertyCalls++;
    return sProperties[sPropertyCalls - 1];
}

u8 UnitStateE9Bit0(const u8 *unit)
{
    assert(unit == (const u8 *)sRecord);
    assert(sPropertyCalls == 3);
    assert(sFlagCalls == 0);
    assert(sRecord->fields.unknown34[9] == sExpectedModeAtFlag);
    sFlagCalls++;
    if (sMutateMode)
        sRecord->fields.unknown34[9] = sModeAfterFlag;
    return sFlag;
}
#else
u32 GetJobProperty(u8 job, u8 fallback, u8 property)
{
    assert(job == sExpectedJob);
    assert(fallback == sExpectedFallback);
    assert(sPropertyCalls < 3);
    assert(property == (sPropertyCalls == 0 ? 34
                         : sPropertyCalls == 1 ? 35 : 31));
    sPropertyCalls++;
    return sProperties[sPropertyCalls - 1];
}

u8 UnitField19IsZero(const struct UnitFieldView *unit)
{
    (void)unit;
    return 0;
}
#endif

u32 SumUnitEquipmentProperty(const struct UnitFieldView *unit,
                             u32 selector)
{
    assert(unit == &sRecord->fields);
    assert(selector == 17);
    assert(sPropertyCalls == 3);
#ifndef REAL_COMPOSITION
    assert(sFlagCalls == 1);
#endif
    assert(sEquipmentCalls == 0);
    sEquipmentCalls++;
    return sEquipmentSum;
}

static void init_record(struct GuardedRecord *guarded, u8 mode)
{
    unsigned i;
    memset(guarded, 0, sizeof(*guarded));
    for (i = 0; i < sizeof(guarded->before); i++)
        guarded->before[i] = (u8)(0x31 + i);
    memset(&guarded->record, 0xA5, sizeof(guarded->record));
    for (i = 0; i < sizeof(guarded->after); i++)
        guarded->after[i] = (u8)(0x72 + i);
    guarded->record.fields.unknown34[9] = mode;
}

static void reset_control(struct UnitRecord *record, u32 first, u32 second,
                          u32 third, u8 flag, u32 sum, int mutate_mode,
                          u8 expected_mode, u8 mode_after)
{
    sRecord = record;
    sProperties[0] = first;
    sProperties[1] = second;
    sProperties[2] = third;
    sPropertyCalls = 0;
    sFlag = flag;
    sFlagCalls = 0;
    sExpectedModeAtFlag = expected_mode;
    sModeAfterFlag = mode_after;
    sMutateMode = mutate_mode;
    sEquipmentSum = sum;
    sEquipmentCalls = 0;
}

#ifndef REAL_COMPOSITION
static void run_controlled_case(u8 input_mode, u32 first, u32 second,
                                u32 third, u8 flag, u32 sum,
                                int mutate_mode, u8 mode_after,
                                u8 expected_first, u8 expected_second,
                                u8 expected_third)
{
    struct GuardedRecord actual, expected;
    u8 flag_mode = input_mode;

    init_record(&actual, input_mode);
    expected = actual;
    if (mutate_mode) {
        expected.record.fields.unknown34[9] = mode_after;
        flag_mode = input_mode;
    }
    expected.record.unknownFC[0] = expected_first;
    expected.record.unknownFC[1] = expected_second;
    expected.record.unknownFC[2] = expected_third;
    expected.record.unknownFC[3] = (u8)~expected_third;
    reset_control(&actual.record, first, second, third, flag, sum,
                  mutate_mode, flag_mode, mode_after);
    UpdateUnitDerivedBytes(&actual.record);
    assert(sPropertyCalls == 3);
    assert(sFlagCalls == 1);
    assert(sEquipmentCalls == 1);
    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
}

static void controlled_cases(void)
{
    /* Mode 2 increments 0xFF to zero. */
    run_controlled_case(2, 0, 0, 0xff, 0, 0, 0, 0, 0, 0, 0);
    /* Mode 2 increments 0xFE to 0xFF, then low equipment byte 3 wraps to 2. */
    run_controlled_case(2, 0x123400ffu, 0x45670002u, 0xabcdeffeu,
                        0, 3, 0, 0, 0xff, 2, 2);
    /* Mode 3 adds two before the 0xFF equipment byte: 0xFE + 2 + 0xFF = 0xFF. */
    run_controlled_case(3, 9, 10, 0xabcdefFEu, 0, 0x123456ffu,
                        0, 0, 9, 10, 0xff);
    run_controlled_case(4, 8, 9, 7, 0, 0, 0, 0, 8, 5, 7);
    run_controlled_case(5, 8, 1, 7, 0, 0, 0, 0, 8, 3, 7);
    run_controlled_case(5, 8, 2, 7, 0, 0, 0, 0, 8, 3, 7);
    run_controlled_case(5, 8, 0, 7, 0, 0, 0, 0, 8, 0, 7);
    run_controlled_case(5, 8, 3, 7, 0, 0, 0, 0, 8, 3, 7);
    run_controlled_case(5, 8, 255, 7, 0, 0, 0, 0, 8, 255, 7);
    /* Status forcing precedes mode 6, whose case then overrides first to 3. */
    run_controlled_case(6, 0x100u, 4, 5, 1, 0, 0, 0, 3, 4, 5);
    /* A nonzero flag forces first on in the default switch case. */
    run_controlled_case(0, 0, 6, 7, 1, 0, 0, 0, 1, 6, 7);
    /* Other nonzero flag values are truthy, and an existing first value stays 1. */
    run_controlled_case(1, 0, 6, 7, 2, 0, 0, 0, 1, 6, 7);
    run_controlled_case(0, 0x101, 6, 7, 1, 0, 0, 0, 1, 6, 7);
    run_controlled_case(0, 2, 3, 4, 0, 0, 0, 0, 2, 3, 4);
    run_controlled_case(1, 2, 3, 4, 0, 0, 0, 0, 2, 3, 4);
    run_controlled_case(7, 2, 3, 4, 0, 0, 0, 0, 2, 3, 4);
    run_controlled_case(255, 2, 3, 4, 0, 0, 0, 0, 2, 3, 4);
    /* The mode is loaded after the flag callback, which changes 0 to case 4. */
    run_controlled_case(0, 2, 3, 4, 0, 0, 1, 4, 2, 5, 4);
}
#endif

static void forwarding_cases(void)
{
    struct GuardedRecord actual, expected;
    init_record(&actual, 7);
    actual.record.fields.unknownE8[1] = 0;
    actual.record.unknownFC[2] = 0x21;
    expected = actual;
    expected.record.unknownFC[0] = 0;
    expected.record.unknownFC[1] = 0;
    expected.record.unknownFC[2] = 0x80;
    expected.record.unknownFC[3] = 0x7f;
    reset_control(&actual.record, 0, 0, 0, 0, 0, 0, 7, 0);
#ifndef REAL_COMPOSITION
    sEquipmentSum = 0x80;
    assert(RefreshAndReadUnitDerivedByte(&actual.record) == -128);
    assert(sPropertyCalls == 3 && sFlagCalls == 1 && sEquipmentCalls == 1);
#else
    /* Real updater composition is covered below; initialize its live getters. */
    sProperties[0] = 0;
    sProperties[1] = 0;
    sProperties[2] = 0x80;
    sExpectedJob = actual.record.fields.bytes04[1];
    sExpectedFallback = actual.record.fields.bytes04[3];
    assert(RefreshAndReadUnitDerivedByte(&actual.record) == -128);
    assert(sPropertyCalls == 3 && sEquipmentCalls == 1);
#endif
    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
}

#ifdef REAL_COMPOSITION
static void real_dependency_composition(void)
{
    struct GuardedRecord actual, expected;
    init_record(&actual, 6);
    actual.record.fields.bytes04[1] = 11;
    actual.record.fields.bytes04[3] = 12;
    actual.record.fields.unknownE8[1] = 0; /* E9 bit 0 is clear. */
    expected = actual;
    expected.record.unknownFC[0] = 3;
    expected.record.unknownFC[1] = 22;
    expected.record.unknownFC[2] = 33;
    expected.record.unknownFC[3] = (u8)~33;
    sExpectedJob = 11;
    sExpectedFallback = 12;
    sProperties[0] = 0x100u;
    sProperties[1] = 22;
    sProperties[2] = 33;
    sRecord = &actual.record;
    sPropertyCalls = 0;
    sFlagCalls = 0;
    sEquipmentSum = 0;
    sEquipmentCalls = 0;
    UpdateUnitDerivedBytes(&actual.record);
    assert(sPropertyCalls == 3);
    assert(sEquipmentCalls == 1);
    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
}

static void real_truthy_status_composition(void)
{
    struct GuardedRecord actual, expected;
    init_record(&actual, 4);
    actual.record.fields.bytes04[1] = 11;
    actual.record.fields.bytes04[3] = 12;
    actual.record.fields.unknownE8[1] = 1; /* E9 bit 0 is set. */
    expected = actual;
    expected.record.unknownFC[0] = 1;
    expected.record.unknownFC[1] = 5;
    expected.record.unknownFC[2] = 33;
    expected.record.unknownFC[3] = (u8)~33;
    sExpectedJob = 11;
    sExpectedFallback = 12;
    sProperties[0] = 0;
    sProperties[1] = 22;
    sProperties[2] = 33;
    sRecord = &actual.record;
    sPropertyCalls = 0;
    sFlagCalls = 0;
    sEquipmentSum = 0;
    sEquipmentCalls = 0;
    UpdateUnitDerivedBytes(&actual.record);
    assert(sPropertyCalls == 3);
    assert(sEquipmentCalls == 1);
    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
}
#endif

int main(void)
{
    assert(sizeof(struct UnitFieldView) == 0xFC);
    assert(sizeof(struct UnitRecord) == RECORD_SIZE);
    assert(__builtin_offsetof(struct UnitRecord, unknownFC) == 0xFC);
    assert(__builtin_offsetof(struct UnitRecord, unknownFC) + 3 == 0xFF);
    assert(__builtin_offsetof(struct UnitRecord, fields.unknown34) + 9 == 0x3D);
#ifndef REAL_COMPOSITION
    controlled_cases();
#else
    real_dependency_composition();
    real_truthy_status_composition();
#endif
    forwarding_cases();
    return 0;
}
