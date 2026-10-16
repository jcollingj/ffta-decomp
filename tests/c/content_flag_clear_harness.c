#include <assert.h>
#include <string.h>

#include "../../include/types.h"

void ClearContentFlag21FIfSet(void);

#define GAME_FLAGS_SIZE 0x200
u8 gGameFlags[GAME_FLAGS_SIZE];

#ifdef USE_REAL_GAME_FLAGS
u8 GetGameFlag(u16 flag);
void SetGameFlag(u16 flag, u16 value);

static void TestRealFlagApis(void)
{
    u8 expected[GAME_FLAGS_SIZE];
    unsigned initial;

    for (initial = 0; initial <= 0xFF; initial++) {
        memset(gGameFlags, 0xA5, sizeof(gGameFlags));
        gGameFlags[0x21F / 8] = (u8)initial;
        memcpy(expected, gGameFlags, sizeof(expected));
        expected[0x21F / 8] &= (u8)~(1u << (0x21F & 7));

        ClearContentFlag21FIfSet();

        assert(memcmp(gGameFlags, expected, sizeof(expected)) == 0);
        assert(GetGameFlag(0x21F) == 0);
    }

    /* A set bit and its neighboring bits are preserved independently. */
    memset(gGameFlags, 0x5A, sizeof(gGameFlags));
    gGameFlags[0x21F / 8] &= (u8)~(1u << (0x21F & 7));
    SetGameFlag(0x21F, 1);
    assert(GetGameFlag(0x21F) == 1);
    ClearContentFlag21FIfSet();
    assert(GetGameFlag(0x21F) == 0);
    assert(gGameFlags[0x21F / 8] == 0x5A);
}

int main(void)
{
    TestRealFlagApis();
    return 0;
}
#else
static u8 getterResult;
static unsigned getCalls;
static unsigned setCalls;
static u16 lastGetFlag;
static u16 lastSetFlag;
static u16 lastSetValue;
static char eventLog[3];
static u8 expectedAtSet[GAME_FLAGS_SIZE];

u8 GetGameFlag(u16 flag)
{
    assert(getCalls == 0);
    assert(setCalls == 0);
    getCalls++;
    lastGetFlag = flag;
    eventLog[0] = 'G';
    return getterResult;
}

void SetGameFlag(u16 flag, u16 value)
{
    assert(setCalls == 0);
    assert(getCalls == 1);
    setCalls++;
    lastSetFlag = flag;
    lastSetValue = value;
    eventLog[1] = 'S';
    eventLog[2] = '\0';
    assert(memcmp(gGameFlags, expectedAtSet, sizeof(gGameFlags)) == 0);
}

static void ResetCase(void)
{
    getCalls = 0;
    setCalls = 0;
    lastGetFlag = 0xFFFF;
    lastSetFlag = 0xFFFF;
    lastSetValue = 0xFFFF;
    memset(eventLog, 0, sizeof(eventLog));
}

static void TestAllGetterByteValues(void)
{
    u8 before[GAME_FLAGS_SIZE];
    unsigned value;

    for (value = 0; value <= 0xFF; value++) {
        memset(gGameFlags, (u8)(value ^ 0xA5), sizeof(gGameFlags));
        memcpy(before, gGameFlags, sizeof(before));
        memcpy(expectedAtSet, before, sizeof(expectedAtSet));
        getterResult = (u8)value;
        ResetCase();

        ClearContentFlag21FIfSet();

        assert(getCalls == 1);
        assert(lastGetFlag == 0x21F);
        assert(memcmp(gGameFlags, before, sizeof(before)) == 0);
        if (value == 0) {
            assert(setCalls == 0);
            assert(eventLog[0] == 'G' && eventLog[1] == '\0');
        } else {
            assert(setCalls == 1);
            assert(lastSetFlag == 0x21F && lastSetValue == 0);
            assert(eventLog[0] == 'G' && eventLog[1] == 'S' &&
                   eventLog[2] == '\0');
        }
    }
}

int main(void)
{
    TestAllGetterByteValues();
    return 0;
}
#endif
