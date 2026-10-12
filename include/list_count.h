#ifndef FFTA_LIST_COUNT_H
#define FFTA_LIST_COUNT_H
#include "types.h"
#include "container.h"

/* Node pointer offset is established; the two leading node words remain unknown. */
struct ListCountNode {
    u32 unknown00;
    u32 unknown04;
    struct ListCountNode *next;
};

u32 CountLinkedNodes(const struct Container *owner);

/* On target the pointer field offsets are checked by the matching build. */
typedef char ListCountTargetNodeNextOffset[
    sizeof(void *) != 4 || sizeof(struct ListCountNode) == 12 ? 1 : -1];

#endif
