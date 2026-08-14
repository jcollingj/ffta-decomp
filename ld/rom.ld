OUTPUT_ARCH(arm)
ENTRY(_start)

SECTIONS
{
    . = 0x08000000;
    .text : {
        *(.text)
    }
    /DISCARD/ : { *(*) }
}
