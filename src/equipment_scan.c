#include "equipment_scan.h"
u8 ScanEquipmentMask81(struct EquipmentView *unit, u8 clear)
{
    int found = 0;
    int slot;
    for (slot = 0; slot < 5; slot++) {
        u16 item = unit->items[slot];
        if (!item) continue;
        if ((InspectEquipment(unit, item, slot, 1) & 0x81) == 0x81) {
            if (clear) {
                ReplaceEquipment(unit, 0, slot);
                slot = 0;
            }
            found = 1;
        }
    }
    return found;
}
