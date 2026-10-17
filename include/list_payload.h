#ifndef FFTA_LIST_PAYLOAD_H
#define FFTA_LIST_PAYLOAD_H

/*
 * The target uses ILP32 and the host test environment uses LP64. In either
 * ABI, unsigned long can hold a complete object-pointer representation. The
 * equal-width assertion deliberately rejects LLP64 and other unsupported
 * pointer/integer layouts instead of truncating payload pointers.
 */
typedef unsigned long ListPayload;

typedef char ListPayloadSupportedAbi[
    ((sizeof(void *) == 4 && sizeof(unsigned long) == 4) ||
     (sizeof(void *) == 8 && sizeof(unsigned long) == 8)) ? 1 : -1];
typedef char ListPayloadNativePointerWidth[
    sizeof(ListPayload) == sizeof(void *) ? 1 : -1];
typedef char ListPayloadTargetWidth[
    sizeof(void *) != 4 || sizeof(ListPayload) == 4 ? 1 : -1];

#endif
