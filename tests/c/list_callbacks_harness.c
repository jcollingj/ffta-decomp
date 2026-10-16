#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "../../include/list_callbacks.h"
#include "../../include/tree_count.h"

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
    struct TreeCountNode node;
    struct TreeCountNode before;
    void *output;
    unsigned flag;

    for (flag = 0; flag <= 0xFFFF; flag++) {
        memset(&node, 0xA5, sizeof(node));
        node.flags = 0xA5A50000u | flag;
        before = node;
        output = (void *)(uintptr_t)0x1234;
        if (flag & 1) {
            assert(ListCallbackSelectClear(&node, &output) == 1);
            assert(output == (void *)(uintptr_t)0x1234);
        } else {
            assert(ListCallbackSelectClear(&node, &output) == 0);
            assert(output == &node);
        }
        assert(memcmp(&node, &before, sizeof(node)) == 0);
    }

    /* A nonzero upper pattern cannot make a clear low bit look set. */
    node.flags = 0xFFFFFFFEu;
    output = (void *)(uintptr_t)0x4321;
    assert(ListCallbackSelectClear(&node, &output) == 0);
    assert(output == &node);
}

static void TestClearOnlyBitZero(void)
{
    struct TreeCountNode node;
    struct TreeCountNode before;
    u32 value;

    for (value = 0; value <= 0xFFFF; value++) {
        memset(&node, 0x5A, sizeof(node));
        node.flags = 0xA5A50000u | value;
        before = node;
        assert(ListCallbackClearFlag0(&node, 0) == 1);
        before.flags &= ~1u;
        assert(memcmp(&node, &before, sizeof(node)) == 0);
    }
}

int main(void)
{
    TestAppendOrderAndWrap();
    TestSelectAllLowFlags();
    TestClearOnlyBitZero();
    return 0;
}
