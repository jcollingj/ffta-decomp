#ifndef FFTA_UNIT_REFERENCE_H
#define FFTA_UNIT_REFERENCE_H
#include "unit_pool.h"
/* Four-byte encoded index, permitted at unaligned source addresses. */
struct EncodedUnitReference { u32 value; } __attribute__((packed));
typedef char EncodedUnitReferenceSize[(sizeof(struct EncodedUnitReference) == 4) ? 1 : -1];
/* Pointee format and table bounds are not recovered here. */
extern const u8 *const gUnitReferenceTable0[];
extern const u8 *const gUnitReferenceTable1[];
extern u8 gUnitReferenceFallback[];
const u8 *ResolveUnitReference(const struct UnitRecord *unit);
const u8 *ResolveEncodedUnitReference(const struct EncodedUnitReference *encoded);
#endif
