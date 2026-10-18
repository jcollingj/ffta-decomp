#include "content_position.h"

void SetContentEntryPosition(struct ContentConsumerEntry *entry,
                             signed int x, signed int y)
{
    signed char x_byte;
    signed char y_byte;
    u32 value;

    x_byte = (signed char)x;
    y_byte = (signed char)y;
    entry->position08 = (signed short)(x_byte * 32 + 16);
    entry->position0C = (signed short)(y_byte * 32 + 16);
    value = ReadContentUnitCoordinateValue(entry, x_byte, y_byte);
    entry->coordinateValue0A = (u16)(value << 4);
    entry->object->bytesF1[5] = (u8)x_byte;
    entry->object->bytesF1[6] = (u8)y_byte;
}

asm(".align 2, 0");
