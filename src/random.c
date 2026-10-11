#include "random.h"
extern u32 gRandomState;
u16 NextRandom15(void)
{
    gRandomState = gRandomState * 0x41c64e6d + 12345;
    return (gRandomState >> 16) & 0x7fff;
}
void SetRandomState(u32 state) { gRandomState = state; }
u32 *GetRandomStateAddress(void) { return &gRandomState; }
