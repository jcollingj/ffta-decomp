#ifndef FFTA_LIST_COUNT_WRAPPERS_H
#define FFTA_LIST_COUNT_WRAPPERS_H
#include "list_count.h"

/* The 16-byte prefix and all three owner pointers have unknown meanings. */
struct ListCountContext {
    u8 unknown00[16];
    struct Container *unknown10;
    struct Container *unknown14;
    struct Container *unknown18;
};

u32 CountContextListAt10(const struct ListCountContext *context);
u32 CountContextListAt14(const struct ListCountContext *context);
u32 CountContextListAt18(const struct ListCountContext *context);

/* Target pointers are four bytes; observed fields occupy offsets 0x10/0x14/0x18. */
typedef char ListCountContextTargetSize[
    sizeof(void *) != 4 || sizeof(struct ListCountContext) == 28 ? 1 : -1];

#endif
