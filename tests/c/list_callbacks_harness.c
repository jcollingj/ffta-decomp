#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "../../include/list_callbacks.h"
#include "../../src/list_callbacks.c"

static void TestAppendOrderAndWrap(void)
{
    uintptr_t values[5] = {0x11, 0x22, 0x33, 0x44, 0x55};
    void *output[7];
    struct ListCallbackState state;
    unsigned i;

    for (i = 0; i < 7; i++)
        output[i] = (void *)(uintptr_t)(0x100 + i);
    state.cursor = output + 1;
    state.count = 0xFFFFFFFEu;
    assert(ListCallbackAppend((void *)values[0], &state) == 1);
    assert(ListCallbackAppend((void *)values[1], &state) == 1);
    assert(ListCallbackAppend((void *)values[2], &state) == 1);
    assert(ListCallbackAppend((void *)values[3], &state) == 1);
    assert(state.count == 2);
    assert(state.cursor == output + 5);
    for (i = 0; i < 4; i++)
        assert(output[i + 1] == (void *)values[i]);
    assert(output[0] == (void *)(uintptr_t)0x100);
    assert(output[5] == (void *)(uintptr_t)0x105);
    assert(output[6] == (void *)(uintptr_t)0x106);

    /* A second order confirms that the callback preserves caller order. */
    state.cursor = output + 1;
    state.count = 17;
    assert(ListCallbackAppend((void *)values[3], &state) == 1);
    assert(ListCallbackAppend((void *)values[1], &state) == 1);
    assert(ListCallbackAppend((void *)values[4], &state) == 1);
    assert(output[1] == (void *)values[3]);
    assert(output[2] == (void *)values[1]);
    assert(output[3] == (void *)values[4]);
    assert(output[4] == (void *)values[3]);
    assert(output[5] == (void *)(uintptr_t)0x105);
    assert(state.cursor == output + 4);
    assert(state.count == 20);
}

static void TestSelectAllLowFlags(void)
{
    u32 node[8];
    u32 before[8];
    const u32 *output;
    unsigned flag;

    for (flag = 0; flag <= 0xFFFF; flag++) {
        unsigned i;
        for (i = 0; i < 8; i++)
            node[i] = 0xA5000000u + i * 0x10101u;
        node[5] = 0xA5A50000u | flag;
        memcpy(before, node, sizeof(node));
        output = (const u32 *)(uintptr_t)0x1234;
        if (flag & 1) {
            assert(ListCallbackSelectClear(node, &output) == 1);
            assert(output == (const u32 *)(uintptr_t)0x1234);
        } else {
            assert(ListCallbackSelectClear(node, &output) == 0);
            assert(output == node);
        }
        assert(memcmp(node, before, sizeof(node)) == 0);
    }

    /* A nonzero upper pattern cannot make a clear low bit look set. */
    node[5] = 0xFFFFFFFEu;
    output = (const u32 *)(uintptr_t)0x4321;
    assert(ListCallbackSelectClear(node, &output) == 0);
    assert(output == node);
}

static void TestClearOnlyBitZero(void)
{
    u32 node[8];
    u32 before[8];
    u32 value;

    for (value = 0; value <= 0xFFFF; value++) {
        unsigned i;
        for (i = 0; i < 8; i++)
            node[i] = 0x5A000000u + i * 0x12345u;
        node[5] = 0xA5A50000u | value;
        memcpy(before, node, sizeof(node));
        assert(ListCallbackClearFlag0(node) == 1);
        before[5] &= ~1u;
        assert(memcmp(node, before, sizeof(node)) == 0);
    }
}

int main(void)
{
    TestAppendOrderAndWrap();
    TestSelectAllLowFlags();
    TestClearOnlyBitZero();
    return 0;
}
