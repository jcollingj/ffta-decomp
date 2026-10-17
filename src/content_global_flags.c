#include "content_globals.h"

void ClearContentGlobalFlag800AndSet1000(void)
{
    gContentGlobalPointerOwner->flagsD0 &= 0xFFFFF7FF;
    gContentGlobalPointerOwner->flagsD0 |= 0x00001000;
}

void AddContentGlobalFlags8800(void)
{
    gContentGlobalPointerOwner->flagsD0 |= 0x00008800;
}

void ClearContentGlobalFlags8800(void)
{
    gContentGlobalPointerOwner->flagsD0 &= 0xFFFF77FF;
}

asm(".align 2, 0");
