/* Matching recovery for the generic content-index RAM accessors.
 * Original bounds: 0x0800A018..0x0800A024 and 0x0800A024..0x0800A040.
 * Both literal words resolve independently to 0x020020D0. */
#include "types.h"

struct ContentIndexState {
    u8 unknown00[0xa0];
    u16 currentIndex;
    u16 fallbackIndex;
};
extern struct ContentIndexState gContentState;

u16 ReadCurrentContentIndex(void)
{
    return gContentState.currentIndex;
}

/* If the optional index at +0xa2 is zero, use the current index at +0xa0. */
u16 ReadContentIndexOrFallback(void)
{
    u16 index = gContentState.fallbackIndex;
    if (index == 0) {
        index = gContentState.currentIndex;
    }
    return index;
}
