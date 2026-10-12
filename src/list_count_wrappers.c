#include "list_count_wrappers.h"

u32 CountContextListAt10(const struct ListCountContext *context)
{
    return CountLinkedNodes(context->unknown10);
}

u32 CountContextListAt14(const struct ListCountContext *context)
{
    return CountLinkedNodes(context->unknown14);
}

u32 CountContextListAt18(const struct ListCountContext *context)
{
    return CountLinkedNodes(context->unknown18);
}

asm(".align 2,0");
