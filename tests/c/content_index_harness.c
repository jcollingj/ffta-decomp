#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "types.h"

#include "../../src/content_index.c"

/* The routines access aligned halfwords at byte offsets 0xa0 and 0xa2. */
struct ContentIndexState gContentState __attribute__((aligned(4)));

static void put_halfword(unsigned offset, u16 value)
{
    memcpy((u8 *)&gContentState + offset, &value, sizeof(value));
}

static u16 get_halfword(unsigned offset)
{
    u16 value;
    memcpy(&value, (u8 *)&gContentState + offset, sizeof(value));
    return value;
}

static void verify_state_unchanged(const u8 *before)
{
    assert(memcmp(before, &gContentState, sizeof(gContentState)) == 0);
}

int main(void)
{
    u8 before[sizeof(gContentState)];
    unsigned value;

    assert(offsetof(struct ContentIndexState, currentIndex) == 0xa0);
    assert(offsetof(struct ContentIndexState, fallbackIndex) == 0xa2);

    /* Independently vary every possible current-index halfword. */
    for (value = 0; value <= 0xffffu; ++value) {
        memset(&gContentState, 0x5a, sizeof(gContentState));
        put_halfword(0xa0, (u16)value);
        put_halfword(0xa2, (u16)(value ^ 0xffffu));
        memcpy(before, &gContentState, sizeof(before));
        assert(ReadCurrentContentIndex() == (u16)value);
        verify_state_unchanged(before);
    }

    /* The nonzero branch returns every nonzero halfword at +0xa2. */
    for (value = 1; value <= 0xffffu; ++value) {
        memset(&gContentState, 0xc3, sizeof(gContentState));
        put_halfword(0xa0, 0x369c);
        put_halfword(0xa2, (u16)value);
        memcpy(before, &gContentState, sizeof(before));
        assert(ReadContentIndexOrFallback() == (u16)value);
        verify_state_unchanged(before);
    }

    /* With +0xa2 zero, exhaust the full fallback halfword domain. */
    for (value = 0; value <= 0xffffu; ++value) {
        memset(&gContentState, 0x96, sizeof(gContentState));
        put_halfword(0xa0, (u16)value);
        put_halfword(0xa2, 0);
        memcpy(before, &gContentState, sizeof(before));
        assert(ReadContentIndexOrFallback() == (u16)value);
        verify_state_unchanged(before);
    }

    /* Pin the zero selector, a nonzero selector, and dissimilar decoys. */
    memset(&gContentState, 0x7e, sizeof(gContentState));
    put_halfword(0xa0, 0xffff);
    put_halfword(0xa2, 0);
    assert(ReadContentIndexOrFallback() == 0xffff);
    put_halfword(0xa0, 0x1234);
    put_halfword(0xa2, 0x8001);
    assert(ReadContentIndexOrFallback() == 0x8001);
    assert(get_halfword(0xa0) == 0x1234);

    return 0;
}
