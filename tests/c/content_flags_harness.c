#include <assert.h>
#include <string.h>

#include "../../src/content_flags.c"
#include "../../src/nonmatching/content_flag_clear.c"

union SaveBacking {
    u32 alignment;
    u8 bytes[0x4000];
};

union SaveBacking saveBacking;
#if defined(__APPLE__)
__asm__(".globl _gSaveData\n.set _gSaveData, _saveBacking");
#else
__asm__(".globl gSaveData\n.set gSaveData, saveBacking");
#endif
#define gSaveData (*(struct ContentFlagSaveView *)saveBacking.bytes)

static u8 flagResult;
static u16 getCalls;
static u16 setCalls;
static u16 lastGetFlag;
static u16 lastSetFlag;
static u16 lastSetValue;
static char events[8];
static unsigned eventCount;
static u16 expectedCopiedValue;
static int checkCopyBeforeSet;

static void Event(char event)
{
    assert(eventCount + 1 < sizeof(events));
    events[eventCount++] = event;
    events[eventCount] = '\0';
}

u8 GetGameFlag(u16 flag)
{
    assert(flag == 0x21F);
    getCalls++;
    lastGetFlag = flag;
    Event('G');
    return flagResult;
}

void SetGameFlag(u16 flag, u16 value)
{
    assert(flag == 0x21F);
    setCalls++;
    lastSetFlag = flag;
    lastSetValue = value;
    Event('S');
    if (checkCopyBeforeSet)
        assert(gSaveData.halfword2192 == expectedCopiedValue);
}

static void ResetCalls(void)
{
    getCalls = 0;
    setCalls = 0;
    lastGetFlag = 0;
    lastSetFlag = 0;
    lastSetValue = 0;
    eventCount = 0;
    memset(events, 0, sizeof(events));
}

static void TestGetterReturnsEveryByte(void)
{
    unsigned value;
    for (value = 0; value <= 0xFF; value++) {
        ResetCalls();
        flagResult = (u8)value;
        assert(GetContentFlag21F() == (u8)value);
        assert(getCalls == 1 && setCalls == 0);
        assert(lastGetFlag == 0x21F);
        assert(strcmp(events, "G") == 0);
    }
}

static void TestCopyAllHalfwordsAndSetAfterCopy(void)
{
    unsigned source;
    u8 expected[sizeof(saveBacking.bytes)];

    memset(saveBacking.bytes, 0xA5, sizeof(saveBacking.bytes));
    for (source = 0; source <= 0xFFFF; source++) {
        gSaveData.halfword2192 = (u16)(source ^ 0x5AA5);
        gSaveData.halfword2194 = (u16)source;
        expectedCopiedValue = (u16)source;
        checkCopyBeforeSet = 1;
        ResetCalls();
        CopySaveHalfword2194To2192AndSetFlag21F();
        assert(gSaveData.halfword2192 == (u16)source);
        assert(gSaveData.halfword2194 == (u16)source);
        assert(getCalls == 0 && setCalls == 1);
        assert(lastSetFlag == 0x21F && lastSetValue == 1);
        assert(strcmp(events, "S") == 0);
    }

    /* A full backing snapshot checks that only the destination halfword
     * changes, including adjacent and otherwise unmodeled sentinel bytes.
     */
    memset(saveBacking.bytes, 0x3C, sizeof(saveBacking.bytes));
    gSaveData.halfword2192 = 0x1357;
    gSaveData.halfword2194 = 0xBEEF;
    memcpy(expected, saveBacking.bytes, sizeof(expected));
    ((struct ContentFlagSaveView *)expected)->halfword2192 =
        ((struct ContentFlagSaveView *)expected)->halfword2194;
    expectedCopiedValue = ((struct ContentFlagSaveView *)expected)->halfword2194;
    checkCopyBeforeSet = 1;
    ResetCalls();
    CopySaveHalfword2194To2192AndSetFlag21F();
    assert(memcmp(saveBacking.bytes, expected, sizeof(expected)) == 0);
    assert(getCalls == 0 && setCalls == 1);
    assert(lastSetFlag == 0x21F && lastSetValue == 1);
    assert(strcmp(events, "S") == 0);
    checkCopyBeforeSet = 0;
}

static void TestClearOnlyWhenGetterIsNonzero(void)
{
    static const u8 values[] = {0, 1, 2, 0x7F, 0x80, 0xFF};
    unsigned i;

    for (i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        ResetCalls();
        flagResult = values[i];
        ClearContentFlag21FIfSet();
        assert(getCalls == 1 && lastGetFlag == 0x21F);
        if (values[i] == 0) {
            assert(setCalls == 0);
            assert(strcmp(events, "G") == 0);
        } else {
            assert(setCalls == 1);
            assert(lastSetFlag == 0x21F && lastSetValue == 0);
            assert(strcmp(events, "GS") == 0);
        }
    }
}

int main(void)
{
    TestGetterReturnsEveryByte();
    TestCopyAllHalfwordsAndSetAfterCopy();
    TestClearOnlyWhenGetterIsNonzero();
    return 0;
}
