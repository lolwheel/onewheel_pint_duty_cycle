MEMORY
{
  FLASH      (rx)  : ORIGIN = 0, LENGTH = 0x300 /* ORIGIN doesn't matter with -fPIC */
  PROHIBITED_YOU_PROBABLY_INTRODUCED_A_GLOBAL_WRITABLE_VARIABLE (rwx) : ORIGIN = 0x20000000, LENGTH = 0
}

SECTIONS
{
    .text : 
    {
        KEEP(*(.bss_requested_len)) /* Requested size of the BSS by this patch */
        KEEP(*(.bss_ptr)) /* A loader/patcher is supposed to update this */
        . = ALIGN(4);
        KEEP(*(.entry)) /* The main function goes here */
        *(.text .text*)
        *(.rodata .rodata*)
        . = ALIGN(4); /* So that we don't need to pad the patches externally */
    } > FLASH

    .data : 
    {
        *(.data .data*)
    } > PROHIBITED_YOU_PROBABLY_INTRODUCED_A_GLOBAL_WRITABLE_VARIABLE /* No global data allowed */
}