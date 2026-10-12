#include "allocator_wrappers.h"
#include "container_create.h"

struct Container *ContainerCreate(void *allocator_owner)
{
    struct Container *container = AllocatorAllocate(allocator_owner, 12);

    container->allocator_owner = allocator_owner;
    container->list_head = 0;
    container->tree_root = 0;
    return container;
}

asm(".align 2,0");
