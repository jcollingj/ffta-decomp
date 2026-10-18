#include <assert.h>
#include <string.h>
#include "unit_clamped_values.h"

#define UNIT_SIZE 0xFC
#define TRACE_CAPACITY 8

struct GuardedUnit {
    u8 before[8];
    struct UnitFieldView unit;
    u8 after[8];
};

struct ItemReply {
    u16 id;
    u8 selector;
    u32 value;
};

static struct UnitFieldView *sLiveUnit;
static u32 sUnitReply;
static unsigned sUnitCalls;
static struct ItemReply sItemReplies[TRACE_CAPACITY];
static unsigned sItemReplyCount;
static unsigned sItemCalls;
static unsigned sMutation;
static char sTrace[TRACE_CAPACITY];
static unsigned sTraceCount;

static void trace(char event)
{
    assert(sTraceCount < TRACE_CAPACITY);
    sTrace[sTraceCount++] = event;
}

u32 GetUnitProperty(const u8 *unit, u8 selector)
{
    assert(unit == (const u8 *)sLiveUnit);
    assert(selector == 32);
    assert(sUnitCalls == 0);
    sUnitCalls++;
    trace('U');
    return sUnitReply;
}

u32 GetItemProperty(u16 id, u8 selector)
{
    const struct ItemReply *reply;
    assert(sItemCalls < sItemReplyCount);
    reply = &sItemReplies[sItemCalls++];
    assert(id == reply->id);
    assert(selector == reply->selector);
    trace('I');

    if (sMutation == 1 && sItemCalls == 1) {
        sLiveUnit->signedD0[1] = 99;
        sLiveUnit->half18[10] = 30;
    } else if (sMutation == 2 && sItemCalls == 1) {
        sUnitReply = 99;
        sLiveUnit->half18[10] = 30;
    }
    return reply->value;
}

static void reset_calls(struct UnitFieldView *unit, u32 unit_reply,
                        const struct ItemReply *items, unsigned item_count,
                        unsigned mutation)
{
    unsigned i;
    assert(item_count <= TRACE_CAPACITY);
    sLiveUnit = unit;
    sUnitReply = unit_reply;
    sUnitCalls = 0;
    sItemReplyCount = item_count;
    sItemCalls = 0;
    sMutation = mutation;
    sTraceCount = 0;
    for (i = 0; i < item_count; i++)
        sItemReplies[i] = items[i];
}

static void init_unit(struct GuardedUnit *guarded)
{
    memset(guarded, 0xA5, sizeof(*guarded));
    memset(&guarded->unit, 0, sizeof(guarded->unit));
    memset(guarded->before, 0x31, sizeof(guarded->before));
    memset(guarded->after, 0x72, sizeof(guarded->after));
}

static void expect_call_trace(unsigned unit_calls, unsigned item_calls)
{
    unsigned i;
    assert(sUnitCalls == unit_calls);
    assert(sItemCalls == item_calls);
    assert(sTraceCount == unit_calls + item_calls);
    for (i = 0; i < sTraceCount; i++) {
        if (unit_calls != 0 && i == 0)
            assert(sTrace[i] == 'U');
        else
            assert(sTrace[i] == 'I');
    }
}

static void expect_unit(const struct GuardedUnit *actual,
                        const struct GuardedUnit *expected)
{
    assert(memcmp(actual, expected, sizeof(*actual)) == 0);
}

int main(void)
{
    struct GuardedUnit actual, expected;
    static const struct ItemReply none[] = {{0, 0, 0}};
    static const struct ItemReply mixed14[] = {
        {11, 14, 2}, {11, 14, 5}, {12, 14, 7}, {13, 14, 11}
    };
    static const struct ItemReply mixed15[] = {
        {11, 15, 2}, {11, 15, 5}, {12, 15, 7}, {13, 15, 11}
    };
    static const struct ItemReply mutation14[] = {
        {10, 14, 4}, {30, 14, 6}
    };
    static const struct ItemReply mutation15[] = {
        {10, 15, 4}, {30, 15, 6}
    };
    static const struct ItemReply neg_wrap14[] = {{20, 14, 8}};
    static const struct ItemReply pos_wrap14[] = {{21, 14, 65535u}};
    static const struct ItemReply sum_overflow14[] = {
        {22, 14, 0xFFFFFFFEu}, {23, 14, 3}
    };
    static const struct ItemReply neg_wrap15[] = {{40, 15, 1}};
    static const struct ItemReply narrow_before_cap15[] = {{41, 15, 32780u}};
    static const struct ItemReply cap15[] = {{42, 15, 2}};
    static const struct ItemReply sum_overflow15[] = {
        {43, 15, 0xFFFFFFFEu}, {44, 15, 3}
    };
    u32 result;

    assert(sizeof(struct UnitFieldView) == UNIT_SIZE);
    assert(__builtin_offsetof(struct UnitFieldView, signedD0) +
           sizeof(signed short) == 0xD2);
    assert(__builtin_offsetof(struct UnitFieldView, half18) +
           9 * sizeof(u16) == 0x2A);
    assert(__builtin_offsetof(struct UnitFieldView, half18) +
           13 * sizeof(u16) == 0x32);

    /* Empty slots skip item reads; selector 32 is read once for value 15. */
    init_unit(&actual);
    actual.unit.signedD0[1] = -15;
    expected = actual;
    reset_calls(&actual.unit, 0xFFFFFFFFu, none, 0, 0);
    assert(ReadUnitEquipmentValue14(&actual.unit) == 0);
    expect_call_trace(0, 0);
    expect_unit(&actual, &expected);

    reset_calls(&actual.unit, 0xFFFFFFFFu, none, 0, 0);
    assert(ReadUnitEquipmentValue15(&actual.unit) == 0);
    expect_call_trace(1, 0);
    expect_unit(&actual, &expected);

    /* Five slots include zero, a repeated ID, mixed IDs and the final slot. */
    init_unit(&actual);
    actual.unit.signedD0[1] = -3;
    actual.unit.half18[9] = 11;
    actual.unit.half18[10] = 0;
    actual.unit.half18[11] = 11;
    actual.unit.half18[12] = 12;
    actual.unit.half18[13] = 13;
    expected = actual;
    reset_calls(&actual.unit, 10, mixed14, 4, 0);
    assert(ReadUnitEquipmentValue14(&actual.unit) == 22);
    expect_call_trace(0, 4);
    expect_unit(&actual, &expected);

    reset_calls(&actual.unit, 10, mixed15, 4, 0);
    assert(ReadUnitEquipmentValue15(&actual.unit) == 35);
    expect_call_trace(1, 4);
    expect_unit(&actual, &expected);

    /* Base snapshots precede callbacks, and later slots are reloaded. */
    init_unit(&actual);
    actual.unit.signedD0[1] = 3;
    actual.unit.half18[9] = 10;
    actual.unit.half18[10] = 20;
    expected = actual;
    expected.unit.signedD0[1] = 99;
    expected.unit.half18[10] = 30;
    reset_calls(&actual.unit, 0, mutation14, 2, 1);
    assert(ReadUnitEquipmentValue14(&actual.unit) == 13);
    expect_call_trace(0, 2);
    expect_unit(&actual, &expected);

    init_unit(&actual);
    actual.unit.half18[9] = 10;
    actual.unit.half18[10] = 20;
    expected = actual;
    expected.unit.half18[10] = 30;
    reset_calls(&actual.unit, 3, mutation15, 2, 2);
    assert(ReadUnitEquipmentValue15(&actual.unit) == 13);
    expect_call_trace(1, 2);
    expect_unit(&actual, &expected);

    /* Value 14 narrows to signed16 before its negative clamp. */
    init_unit(&actual);
    actual.unit.signedD0[1] = 32760;
    actual.unit.half18[9] = 20;
    expected = actual;
    reset_calls(&actual.unit, 0, neg_wrap14, 1, 0);
    assert(ReadUnitEquipmentValue14(&actual.unit) == 0); /* 32768 -> -32768 -> 0 */
    expect_call_trace(0, 1);
    expect_unit(&actual, &expected);

    init_unit(&actual);
    actual.unit.signedD0[1] = -32768;
    actual.unit.half18[9] = 21;
    expected = actual;
    reset_calls(&actual.unit, 0, pos_wrap14, 1, 0);
    assert(ReadUnitEquipmentValue14(&actual.unit) == 32767); /* -32768 + 65535 */
    expect_call_trace(0, 1);
    expect_unit(&actual, &expected);

    init_unit(&actual);
    actual.unit.signedD0[1] = 5;
    actual.unit.half18[9] = 22;
    actual.unit.half18[10] = 23;
    expected = actual;
    reset_calls(&actual.unit, 0, sum_overflow14, 2, 0);
    assert(ReadUnitEquipmentValue14(&actual.unit) == 6); /* FFFFFFFE + 3 wraps to 1 */
    expect_call_trace(0, 2);
    expect_unit(&actual, &expected);

    /* Value 15 narrows to signed16 before [0,100] clamps. */
    init_unit(&actual);
    actual.unit.half18[9] = 40;
    expected = actual;
    reset_calls(&actual.unit, 0x12347FFFu, neg_wrap15, 1, 0);
    assert(ReadUnitEquipmentValue15(&actual.unit) == 0); /* 32767 + 1 -> -32768 */
    expect_call_trace(1, 1);
    expect_unit(&actual, &expected);

    init_unit(&actual);
    actual.unit.half18[9] = 41;
    expected = actual;
    reset_calls(&actual.unit, 32760u, narrow_before_cap15, 1, 0);
    assert(ReadUnitEquipmentValue15(&actual.unit) == 4); /* 65540 narrows before cap */
    expect_call_trace(1, 1);
    expect_unit(&actual, &expected);

    init_unit(&actual);
    actual.unit.half18[9] = 42;
    expected = actual;
    reset_calls(&actual.unit, 99u, cap15, 1, 0);
    assert(ReadUnitEquipmentValue15(&actual.unit) == 100); /* 101 -> cap */
    expect_call_trace(1, 1);
    expect_unit(&actual, &expected);

    init_unit(&actual);
    actual.unit.half18[9] = 43;
    actual.unit.half18[10] = 44;
    expected = actual;
    reset_calls(&actual.unit, 5u, sum_overflow15, 2, 0);
    result = ReadUnitEquipmentValue15(&actual.unit);
    assert(result == 6); /* FFFFFFFE + 3 wraps to 1 */
    expect_call_trace(1, 2);
    expect_unit(&actual, &expected);

    return 0;
}
