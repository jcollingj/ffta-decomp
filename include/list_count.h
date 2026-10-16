#ifndef FFTA_LIST_COUNT_H
#define FFTA_LIST_COUNT_H
#include "types.h"
#include "container.h"

/* Original append/unlink routines establish target fields at offsets 0/4/8. */
struct ListCountNode {
    u32 unknown00;
    struct ListCountNode *previous;
    struct ListCountNode *next;
};

u32 CountLinkedNodes(const struct Container *owner);

/* GCC2.9-compatible target size check; the field declaration order above
 * gives target offsets 0, 4, and 8 when pointers are four bytes. */
typedef char ListCountTargetNodeSize[
    sizeof(void *) != 4 || sizeof(struct ListCountNode) == 12 ? 1 : -1];

#endif
