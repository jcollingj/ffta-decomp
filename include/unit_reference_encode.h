#ifndef FFTA_UNIT_REFERENCE_ENCODE_H
#define FFTA_UNIT_REFERENCE_ENCODE_H
#include "unit_reference.h"
/* Unaligned pointer input; native host pointers remain native-width. */
struct PackedUnitReferencePointer {
    const u8 *value;
} __attribute__((packed));
typedef char PackedUnitReferencePointerSize[
    (sizeof(struct PackedUnitReferencePointer) == sizeof(const u8 *)) ? 1 : -1];
u32 EncodeUnitReferencePointer(const struct PackedUnitReferencePointer *reference);
#endif
