#include "types.h"
/* Table extent and content identities remain under investigation. */
extern const void * const gContentPointers[];
const void *GetContentPointer123(u16 index)
{
    return gContentPointers[index + 123];
}
u16 AddContentIndex1081(u16 index)
{
    return index + 1081;
}
