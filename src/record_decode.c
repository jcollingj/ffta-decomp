#include "record70.h"
/* Zero is preserved; nonzero compact values map to IDs 376 through 630. */
u16 DecodeRecordByte(u8 value)
{
    if (!value) return 0;
    return value + 375;
}
asm(".align 2, 0");
