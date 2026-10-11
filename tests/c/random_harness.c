#include <assert.h>
#include <string.h>
#include "../../src/random.c"
#include "../../src/random_item_count.c"
u32 gRandomState;
u16 gRandomItemSlots[5];
int main(void)
{
    static const u32 seeds[] = {0, 1, 0xffffffff, 123456789};
    static const u32 states[][6] = {
        {12345u,3554416254u,2802067423u,3596950572u,229283573u,3256818826u},
        {1103527590u,2524885223u,662824084u,3295386429u,4182499122u,2516284547u},
        {3191464396u,288979989u,646343466u,3898514715u,571035320u,3997353105u},
        {231794730u,3274429979u,3905459128u,850994577u,3782040822u,2854729975u}
    };
    static const u16 outputs[][6] = {
        {0,21468,9988,22117,3498,16927}, {16838,5758,10113,17515,31051,5627},
        {15929,4409,9862,26718,8713,28226}, {3536,17195,26824,12985,24941,10791}
    };
    unsigned seed, i, mask, count;
    u16 replay[1024], expected[5];
    assert(sizeof(u32) == 4);
    assert(GetRandomStateAddress() == &gRandomState);
    for (seed = 0; seed < 4; seed++) {
        SetRandomState(seeds[seed]);
        assert(gRandomState == seeds[seed]);
        for (i = 0; i < 6; i++) {
            assert(NextRandom15() == outputs[seed][i]);
            assert(gRandomState == states[seed][i]);
        }
        SetRandomState(seeds[seed]);
        for (i = 0; i < 1024; i++) {
            replay[i] = NextRandom15();
            assert(replay[i] <= 32767);
        }
        *GetRandomStateAddress() = seeds[seed];
        for (i = 0; i < 1024; i++) assert(NextRandom15() == replay[i]);
    }
    for (mask = 0; mask < 32; mask++) {
        count = 0;
        for (i = 0; i < 5; i++) {
            expected[i] = mask & (1u << i) ? (i & 1 ? 1 : 65535) : 0;
            gRandomItemSlots[i] = expected[i];
            count += expected[i] != 0;
        }
        assert(CountRandomItemSlots() == count);
        assert(memcmp(expected, gRandomItemSlots, sizeof(expected)) == 0);
    }
    return 0;
}
