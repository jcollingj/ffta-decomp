/* Work in progress: not linked or counted as recovered.
 * Original region 0xCABA8-0xCAE2C: 644 bytes; current probe: 616 bytes.
 * Behavior still needs dedicated verification; resume from docs/WORK_LOG.md.
 */
#include "equipment_scan.h"
#include "unit.h"
#include "item.h"
extern void (*const gClearMemory)(void *, u32);
extern void (*const gCopyRecordMemory)(void *, const void *, u32);
extern u8 ReadAbilitySlotBValue(const u8 *);
extern u32 GetUnitProperty(const u8 *, u8);
extern const u32 gEquipmentClassMasks[];
static inline u8 Allowed(const u8 *unit, u16 item, u8 support)
{
    u32 mask = gEquipmentClassMasks[GetUnitProperty(unit, 36)];
    u8 category = GetItemProperty(item, 3);
    if (category == 22) return unit[6] == 4;
    if (!(mask & (1u << (category - 1)))) {
        if (category != 20 || support != 15) return 0;
    }
    if (category == 25 || category == 23) {
        if (item == 311 || item == 316 || item == 287) {
            if (unit[6] != 4) return 0;
        }
    }
    return 1;
}
u8 CheckEquipmentSet(struct EquipmentView *unit, const u16 *proposed, u8 selected, u8 flags)
{
    u16 items[5];
    u8 counts[8];
    u8 support;
    int i;
    gClearMemory(counts, 8);
    gCopyRecordMemory(items, proposed, 10);
    support = ReadAbilitySlotBValue((const u8 *)unit);
    if (flags & 4) support = 0;
    for (i = 0; i < 5; i++) {
        u16 item = items[i];
        if (item) {
            u8 kind;
            if (!Allowed((const u8 *)unit, item, support)) goto invalid;
            kind = GetItemProperty(item, 6);
            switch (kind) {
            case 1:
                if ((counts[1] & 127) > 1 || counts[2]) goto invalid;
                if (support == 7 && ((GetItemProperty(item, 7) >> 1) & 1)) {
                    counts[2]++;
                    break;
                }
                if ((u16)(item - 1) <= 251) {
                    if (counts[1] & 128) {
                        if ((counts[1] & 127) > 1) goto invalid;
                    } else if ((counts[1] & 127) != 0 && support != 8) goto invalid;
                } else {
                    u8 job = ReadUnitField((const struct UnitFieldView *)unit, 4);
                    if (counts[1] & 128) goto invalid;
                    if ((u8)(job - 2) <= 1 || job == 13 || job == 27 || job == 28 || job == 37 ||
                        job == 112 || job == 113 || job == 114 || job == 115 || job == 83 || support == 15)
                        counts[1] |= 128;
                    else goto invalid;
                }
                counts[1]++;
                break;
            case 2:
                if (counts[2]) goto invalid;
                if (counts[1]) {
                    if (!(counts[1] & 128)) goto invalid;
                    {
                        u8 job = ReadUnitField((const struct UnitFieldView *)unit, 4);
                        if ((counts[1] & 127) > 1) goto invalid;
                        if ((u8)(job - 2) > 1 && job != 13) goto invalid;
                        if (support != 6) goto invalid;
                    }
                    counts[2]++;
                } else if (support == 6) counts[1]++;
                else counts[2]++;
                break;
            case 7: counts[7]++; break;
            case 3: case 4: case 5: case 6:
                if (counts[kind]) goto invalid;
                counts[kind]++;
                break;
            }
        }
        continue;
invalid:
        return i == (signed char)selected ? 0x81 : 1;
    }
    return 0;
}
asm(".align 2, 0");
