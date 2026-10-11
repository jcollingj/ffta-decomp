#ifndef FFTA_RANDOM_H
#define FFTA_RANDOM_H
#include "types.h"
u16 NextRandom15(void);
void SetRandomState(u32 state);
u32 *GetRandomStateAddress(void);
#endif
