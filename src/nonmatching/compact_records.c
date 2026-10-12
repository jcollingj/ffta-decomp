#include "types.h"

extern u8 gSaveSection21A0[];
extern void (*const gCopyRecordMemory)(void *, const void *, u32);
extern void (*const gClearMemory)(void *, u32);

/* Recovered from 0xCAA74..0xCAB22 (176-byte block including two padding bytes).
 * The original counts occupied records by byte zero across all 64 slots. A zero
 * count returns immediately, without invoking either memory callback. For a
 * nonzero count, it advances the scan index only when the current record is
 * occupied. A hole shifts the tail left by one record, clears the vacated
 * final record, then retries the same index. This packs all occupied records
 * stably into the prefix. Trailing empty records after the last occupied one
 * are not visited, so their payload bytes are not necessarily cleared.
 *
 * Control-flow evidence: at 0xCAACA..0xCAAF2 the zero-record path invokes copy
 * and clear; 0xCAAF6 branches directly to 0xCAB0E, bypassing the pointer/index
 * increments at 0xCAB08..0xCAB0C. Only the nonzero branch at 0xCAAC8 reaches
 * those increments. Both paths then use the loop back-edge at 0xCAB10 to
 * 0xCAAC4. The counting loop separately decrements 63 at 0xCAA96 and branches
 * back at 0xCAA9C until all 64 records have been examined.
 */
int CompactSaveRecords64(void)
{
    u8 *records = gSaveSection21A0 + 0x968;
    int occupied = 0;
    int i;
    int holes = 0;
    u8 *destination;
    u8 *source;

    for (i = 63; i >= 0; i--) {
        if (*records != 0)
            occupied++;
        records += 4;
    }
    records = gSaveSection21A0 + 0x968;
    if (occupied == 0)
        return 0;

    destination = records;
    source = records + 4;
    for (i = 0; i < occupied;) {
        if (*destination == 0) {
            u32 length;
            holes++;
            length = (64 - i - holes) * 4;
            gCopyRecordMemory(destination, source, length);
            gClearMemory(records + (64 - holes) * 4, 4);
        } else {
            i++;
            destination += 4;
            source += 4;
        }
    }
    return occupied;
}
