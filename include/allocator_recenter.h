#ifndef FFTA_ALLOCATOR_RECENTER_H
#define FFTA_ALLOCATOR_RECENTER_H

/* Recenter the owner's stored node offset after following its size chain. */
void AllocatorRecenter(void *owner);

#endif
