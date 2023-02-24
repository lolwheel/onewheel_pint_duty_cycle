ENTRY(deriveAndWriteSoc)

MEMORY
{
  FLASH      (rx)  : ORIGIN = 0, LENGTH = 0x300 /* ORIGIN doesn't matter with -fPIC */
  PROHIBITED_YOU_PROBABLY_INTRODUCED_A_GLOBAL_WRITABLE_VARIABLE (rwx) : ORIGIN = 0x20000000, LENGTH = 0
}

SECTIONS
{
    .text : 
    {
        KEEP(*(.bss_requested_len))
        KEEP(*(.bss_ptr))
        . = ALIGN(4);
        *(.text .text*)
        *(.rodata .rodata*)
    } > FLASH

    .data : 
    {
        *(.data .data*)
    } > PROHIBITED_YOU_PROBABLY_INTRODUCED_A_GLOBAL_WRITABLE_VARIABLE /* No global data allowed */
}