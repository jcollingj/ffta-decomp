#include "content_position.h"

u32 ReadContentUnitCoordinateValue(struct ContentConsumerEntry *entry,
                                   signed int x, signed int y)
{
    return GetContentCoordinateQueryValue(entry->object,
                                          (signed char)x,
                                          (signed char)y);
}

asm(".align 2, 0");
