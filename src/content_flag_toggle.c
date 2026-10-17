#include "content_flag_toggle.h"

#include "content_globals.h"

void UpdateContentGlobalFlag2000(u8 value)
{
    struct ContentGlobalPointerOwner *owner;

    owner = gContentGlobalPointerOwner;
    if (value != 0) {
        owner->flagsD0 &= ~0x2000;
    } else {
        owner->flagsD0 |= 0x2000;
    }
}

asm(".align 2, 0");
