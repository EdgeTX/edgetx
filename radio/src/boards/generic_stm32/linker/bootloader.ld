/*
   Generic bootloader linker script for STM32
*/

INCLUDE definitions.ld

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    KEEP(*(.version))
    KEEP(*(.bootversiondata))

    INCLUDE common_text.ld

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  INCLUDE common_sections.ld
}
