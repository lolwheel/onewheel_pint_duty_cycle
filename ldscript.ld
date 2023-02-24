ENTRY(deriveAndWriteSoc)

SECTIONS
{
    .  = 0x0800F480; /* firmware-5040 is C480 bytes long, 8003000 + C480 = F480 */

    .text : 
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
    }

    .data : 
    {
        . = ALIGN(4);
        *(.data) 
        *(.data*) 
    }
    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
    }

}