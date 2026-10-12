#include "types.h"
extern const void * const gContentPointers[];
const void *GetRemappedContentPointer(u16 index)
{
    if (!index) return gContentPointers[0];
    if (index > 85)
        return gContentPointers[(signed short)(index + 540)];
    return gContentPointers[(signed short)(index + 625)];
}
