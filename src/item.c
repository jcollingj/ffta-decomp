#include "item.h"
extern const struct Item gItems[];
extern u8 ReadSaveByte(u16);

/* Unsigned clamping after signed promotion preserves the original behavior,
 * including negative modifiers when the save-dependent path is enabled. */
u32 GetItemProperty(u16 id, u8 property)
{
    const struct Item *item = &gItems[id];
    switch (property) {
    case 0: return item->name;
    case 1: return item->property01;
    case 2: return item->property02;
    case 3: return item->properties03[0];
    case 4: return item->properties03[1];
    case 5: return item->properties03[2];
    case 6: return item->properties03[3];
    case 7: return item->properties03[4];
    case 8: return item->properties03[5];
    case 9: return item->property09;
    case 10:
        if (ReadSaveByte(13)) return item->modifiers[0];
        else {
            u32 value = item->modifiers[0];
            if (id == 51) value += ReadSaveByte(1);
            if (value > 255) value = 255;
            return value;
        }
    case 11:
        if (ReadSaveByte(13)) return item->modifiers[1];
        else {
            u32 value = item->modifiers[1];
            if (id == 302) value += ReadSaveByte(3);
            if (value > 255) value = 255;
            return value;
        }
    case 12:
        if (ReadSaveByte(13)) return item->modifiers[2];
        else {
            u32 value = item->modifiers[2];
            if (id == 148) value += ReadSaveByte(2);
            if (value > 255) value = 255;
            return value;
        }
    case 13:
        if (ReadSaveByte(13)) return item->modifiers[3];
        else {
            u32 value = item->modifiers[3];
            if (id == 286) value += ReadSaveByte(4);
            if (value > 255) value = 255;
            return value;
        }
    case 14: return item->modifiers[4];
    case 15: return item->modifiers[5];
    case 16: return item->modifiers[6];
    case 17: return item->modifiers[7];
    case 18: return item->ability_set;
    default: return 0;
    }
}
/* Preserve the original zero-filled alignment between compilation regions. */
asm(".align 2, 0");
