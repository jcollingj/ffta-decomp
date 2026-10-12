#ifndef FFTA_CONTAINER_H
#define FFTA_CONTAINER_H

#include "types.h"

struct ListCountNode;
struct TreeCountNode;

/* Shared container owner observed by the list and tree access paths. */
struct Container {
    void *allocator_owner;
    struct ListCountNode *list_head;
    struct TreeCountNode *tree_root;
};

/* The original constructor allocates this 12-byte target object. */
typedef char ContainerTargetSize[
    sizeof(void *) != 4 || sizeof(struct Container) == 12 ? 1 : -1];

#endif
