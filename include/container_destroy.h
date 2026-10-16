#ifndef FFTA_CONTAINER_DESTROY_H
#define FFTA_CONTAINER_DESTROY_H

#include "container.h"

/* Release both owned collections and the container, returning its allocator. */
void *DestroyContainer(struct Container *owner);

#endif
