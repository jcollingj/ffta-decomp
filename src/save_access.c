#include "types.h"
/* These offsets refer to the in-memory save image. Its complete layout and
 * persistence/checksum rules have not yet been recovered. */
/* GNU zero-length tail models an extent that has not yet been recovered. */
struct SaveImage { u8 prefix[0x2030]; u8 bytes[0]; };
extern struct SaveImage gSaveData;
u8 ReadSaveByte(u16 index)
{
    return gSaveData.bytes[index];
}
void WriteSaveByte(u16 index, u8 value)
{
    gSaveData.bytes[index] = value;
}
u8 GetGameFlagFromBuffer(u16 flag, const u8 *save)
{
    const u8 *byte = &save[0x1f70 + flag / 8];
    u8 mask = 1 << (flag & 7);
    if (*byte & mask)
        return 1;
    else
        return 0;
}
void SetGameFlagInBuffer(u16 flag, u16 value, u8 *save)
{
    u8 *byte = &save[0x1f70 + flag / 8];
    u8 mask = 1 << (flag & 7);
    *byte &= ~mask;
    if (value)
        *byte |= mask;
}
