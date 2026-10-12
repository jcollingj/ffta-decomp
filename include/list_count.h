#ifndef FFTA_LIST_COUNT_H
#define FFTA_LIST_COUNT_H
#include "types.h"

/* Partial views. The leading owner word and node words have unknown meaning. */
struct ListCountNode {
    u32 unknown00;
    u32 unknown04;
    struct ListCountNode *next;
};

struct ListCountOwner {
    u32 unknown00;
    struct ListCountNode *head;
};

u32 CountLinkedNodes(const struct ListCountOwner *owner);

/* On the GBA, pointers are four bytes and the observed offsets are exact. */
typedef char ListCountTargetOwnerHeadOffset[
    sizeof(void *) != 4 || sizeof(struct ListCountOwner) == 8 ? 1 : -1];
typedef char ListCountTargetNodeNextOffset[
    sizeof(void *) != 4 || sizeof(struct ListCountNode) == 12 ? 1 : -1];

#endif
