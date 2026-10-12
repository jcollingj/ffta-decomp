#ifndef FFTA_ALLOCATOR_SEARCH_H
#define FFTA_ALLOCATOR_SEARCH_H

#include "types.h"

/* Returns the two nodes selected by the allocator's linked-size search. */
void AllocatorFindNeighbors(void *owner, u16 requested, void **outA, void **outB);

#endif
