#include <assert.h>
#include <string.h>
#include "../../include/item.h"
#include "../../src/random.c"
#include "../../src/nonmatching/random_items.c"

u32 gRandomState;
u16 gRandomItemSlots[5];
#if RANDOM_ITEMS_RETRY_FIXTURE
const struct Item gItems[376] = {
    [0] = {.unknown18 = {1}},
    [212] = {.unknown18 = {2}},
    [309] = {.unknown18 = {3}},
    [7] = {.unknown18 = {0x80}},
    [189] = {.unknown18 = {0xff}},
};
#else
const struct Item gItems[376] = {
    [294] = {.unknown18 = {1}},
    [118] = {.unknown18 = {2}},
    [337] = {.unknown18 = {3}},
    [219] = {.unknown18 = {0x80}},
};
#endif

int main(void)
{
#if RANDOM_ITEMS_RETRY_FIXTURE
    static const u16 expected[5] = {0, 212, 309, 7, 189};
    const u32 seed = 0;
    const u32 final_state = 0xcd1dcf18;
#else
    static const u16 expected[5] = {294, 118, 337, 219, 219};
    const u32 seed = 1;
    const u32 final_state = 0xf94bdf32;
#endif
    unsigned i;

    SetRandomState(seed);
    for (i = 0; i < 5; i++) gRandomItemSlots[i] = 0xa55a;
    FillRandomItemSlots();
    assert(memcmp(expected, gRandomItemSlots, sizeof(expected)) == 0);
    assert(gRandomState == final_state);

    /* A fresh identical seed must reproduce every stored slot and final state. */
    SetRandomState(seed);
    for (i = 0; i < 5; i++) gRandomItemSlots[i] = 0;
    FillRandomItemSlots();
    assert(memcmp(expected, gRandomItemSlots, sizeof(expected)) == 0);
    assert(gRandomState == final_state);
    return 0;
}
