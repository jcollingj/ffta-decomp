/* Byte counters adjacent to state flags; meanings remain provisional. */
#include "unit_status.h"
u8 ReadUnitCounterD8(const u8 *unit) { return unit[0xd8]; }
u8 ReadUnitCounterD9(const u8 *unit) { return unit[0xd9]; }
u8 ReadUnitCounterDA(const u8 *unit) { return unit[0xda]; }
u8 ReadUnitCounterDB(const u8 *unit) { return unit[0xdb]; }
u8 ReadUnitCounterDC(const u8 *unit) { return unit[0xdc]; }
u8 ReadUnitCounterDD(const u8 *unit) { return unit[0xdd]; }
u8 ReadUnitCounterDE(const u8 *unit) { return unit[0xde]; }
u8 ReadUnitCounterDF(const u8 *unit) { return unit[0xdf]; }
u8 ReadUnitCounterE0(const u8 *unit) { return unit[0xe0]; }
u8 ReadUnitCounterE1(const u8 *unit) { return unit[0xe1]; }
u8 ReadUnitCounterE2(const u8 *unit) { return unit[0xe2]; }
u8 ReadUnitCounterE3(const u8 *unit) { return unit[0xe3]; }
u8 ReadUnitCounterE4(const u8 *unit) { return unit[0xe4]; }
u8 ReadUnitCounterE5(const u8 *unit) { return unit[0xe5]; }
u8 ReadUnitCounterE6(const u8 *unit) { return unit[0xe6]; }
void WriteUnitCounterD8(u8 *unit, u8 value) { unit[0xd8] = value; }
void WriteUnitCounterD9(u8 *unit, u8 value) { unit[0xd9] = value; }
void WriteUnitCounterDA(u8 *unit, u8 value) { unit[0xda] = value; }
void WriteUnitCounterDB(u8 *unit, u8 value) { unit[0xdb] = value; }
void WriteUnitCounterDC(u8 *unit, u8 value) { unit[0xdc] = value; }
void WriteUnitCounterDD(u8 *unit, u8 value) { unit[0xdd] = value; }
void WriteUnitCounterDE(u8 *unit, u8 value) { unit[0xde] = value; }
void WriteUnitCounterDF(u8 *unit, u8 value) { unit[0xdf] = value; }
void WriteUnitCounterE0(u8 *unit, u8 value) { unit[0xe0] = value; }
void WriteUnitCounterE1(u8 *unit, u8 value) { unit[0xe1] = value; }
void WriteUnitCounterE2(u8 *unit, u8 value) { unit[0xe2] = value; }
void WriteUnitCounterE3(u8 *unit, u8 value) { unit[0xe3] = value; }
void WriteUnitCounterE4(u8 *unit, u8 value) { unit[0xe4] = value; }
void WriteUnitCounterE5(u8 *unit, u8 value) { unit[0xe5] = value; }
void WriteUnitCounterE6(u8 *unit, u8 value) { unit[0xe6] = value; }
static inline u8 ReadCounter(const u8 *unit, unsigned offset) { return unit[offset]; }
void AddOpposingStateCounter(u8 *unit, u8 first_side)
{
    u8 first = ReadCounter(unit, 0xda);
    u8 second = ReadCounter(unit, 0xdb);
    if ((first & 0x80) || (second & 0x80)) return;
    if (first_side) {
        first += 3;
        if (first > 6) first = 6;
    } else {
        second += 3;
        if (second > 6) second = 6;
    }
    SetOpposingStateCounters(unit, first, second);
}
