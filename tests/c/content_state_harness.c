#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "content_record.h"
#include "content_state.h"

static const u8 sRecordStorage[2][4] = {
    {0x9b, 0x31, 0x42, 0xcd},
    {0x7e, 0xa8, 0xb9, 0x6d}
};

const struct ContentRecord gContentRecords[2] = {
    {0, 0, 0, {0}, &sRecordStorage[0][1], {0}},
    {0, 0, 0, {0}, &sRecordStorage[1][1], {0}}
};

struct ContentIndexState gContentState __attribute__((aligned(4)));

static u8 sFlag253;
static u16 sLastFlag;
static unsigned sFlagCalls;

u8 GetGameFlag(u16 flag)
{
    sLastFlag = flag;
    sFlagCalls++;
    return flag == 0x253 ? sFlag253 : 0;
}

extern u8 ReadContentValue0(void);
extern u8 ReadContentValue1(void);

struct ResultImage {
    u32 leading;
    u8 value0;
    u8 gap[7];
    u8 value1;
    u32 trailing;
};

static void reset_ram(u16 current, u16 fallback, u8 value0, u8 value1,
                      u8 fill)
{
    memset(&gContentState, fill, sizeof(gContentState));
    gContentState.currentIndex = current;
    gContentState.fallbackIndex = fallback;
    gContentState.value0 = value0;
    gContentState.value1 = value1;
    gContentState.unknownA4[0] = 0x61;
    gContentState.unknownA4[1] = 0x92;
}

static void build_expected_ram(u8 expected[0xa8], u16 current, u16 fallback,
                               u8 value0, u8 value1, u8 fill)
{
    memset(expected, fill, 0xa8);
    memcpy(expected + 0xa0, &current, sizeof(current));
    memcpy(expected + 0xa2, &fallback, sizeof(fallback));
    expected[0xa4] = 0x61;
    expected[0xa5] = 0x92;
    expected[0xa6] = value0;
    expected[0xa7] = value1;
}

static void reset_result(struct ResultImage *result)
{
    memset(result, 0xc7, sizeof(*result));
    result->leading = 0x11223344;
    result->value0 = 0x55;
    result->value1 = 0x66;
    result->trailing = 0x88776655;
}

static void test_flag_set_uses_ram_bytes(void)
{
    u8 expected_ram[0xa8];
    u8 expected_storage[sizeof(sRecordStorage)];
    struct ContentRecord expected_records[2];
    struct ResultImage result, expected_result;

    reset_ram(1, 0x9876, 0xe1, 0xf2, 0x4b);
    build_expected_ram(expected_ram, 1, 0x9876, 0xe1, 0xf2, 0x4b);
    memcpy(expected_storage, sRecordStorage, sizeof(expected_storage));
    memcpy(expected_records, gContentRecords, sizeof(expected_records));
    reset_result(&result);
    expected_result = result;
    expected_result.value0 = 0xe1;
    expected_result.value1 = 0xf2;

    sFlag253 = 2;
    sFlagCalls = 0;
    result.value0 = ReadContentValue0();
    assert(result.value0 == 0xe1);
    assert(memcmp(&gContentState, expected_ram, sizeof(expected_ram)) == 0);
    assert(sFlagCalls == 1 && sLastFlag == 0x253);
    result.value1 = ReadContentValue1();
    assert(result.value1 == 0xf2);
    assert(memcmp(&gContentState, expected_ram, sizeof(expected_ram)) == 0);
    assert(sFlagCalls == 2 && sLastFlag == 0x253);
    assert(memcmp(sRecordStorage, expected_storage, sizeof(expected_storage)) == 0);
    assert(memcmp(gContentRecords, expected_records, sizeof(expected_records)) == 0);
    assert(memcmp(&result, &expected_result, sizeof(result)) == 0);
}

static void test_flag_clear_composes_real_current_index_reader(void)
{
    u8 expected_ram[0xa8];
    u8 expected_storage[sizeof(sRecordStorage)];
    struct ContentRecord expected_records[2];
    struct ResultImage result, expected_result;

    reset_ram(0, 1, 0x13, 0x24, 0x3c);
    build_expected_ram(expected_ram, 0, 1, 0x13, 0x24, 0x3c);
    memcpy(expected_storage, sRecordStorage, sizeof(expected_storage));
    memcpy(expected_records, gContentRecords, sizeof(expected_records));
    reset_result(&result);
    expected_result = result;
    expected_result.value0 = 0x31;
    expected_result.value1 = 0x42;

    sFlag253 = 0;
    sFlagCalls = 0;
    result.value0 = ReadContentValue0();
    assert(result.value0 == 0x31);
    assert(memcmp(&gContentState, expected_ram, sizeof(expected_ram)) == 0);
    assert(sFlagCalls == 1 && sLastFlag == 0x253);
    result.value1 = ReadContentValue1();
    assert(result.value1 == 0x42);
    assert(memcmp(&gContentState, expected_ram, sizeof(expected_ram)) == 0);
    assert(sFlagCalls == 2 && sLastFlag == 0x253);
    assert(memcmp(sRecordStorage, expected_storage, sizeof(expected_storage)) == 0);
    assert(memcmp(gContentRecords, expected_records, sizeof(expected_records)) == 0);
    assert(memcmp(&result, &expected_result, sizeof(result)) == 0);
}

int main(void)
{
    assert(sizeof(struct ContentIndexState) == 0xa8);
    assert(offsetof(struct ContentIndexState, currentIndex) == 0xa0);
    assert(offsetof(struct ContentIndexState, fallbackIndex) == 0xa2);
    assert(offsetof(struct ContentIndexState, unknownA4) == 0xa4);
    assert(offsetof(struct ContentIndexState, value0) == 0xa6);
    assert(offsetof(struct ContentIndexState, value1) == 0xa7);
    if (sizeof(void *) == 4) {
        assert(offsetof(struct ContentRecord, sourceEntries) == 4);
        assert(offsetof(struct ContentRecord, values) == 28);
    } else {
        assert(offsetof(struct ContentRecord, sourceEntries) == 8);
        assert(offsetof(struct ContentRecord, values) == 40);
    }
    test_flag_set_uses_ram_bytes();
    test_flag_clear_composes_real_current_index_reader();
    return 0;
}
