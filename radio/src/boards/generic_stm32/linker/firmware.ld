/*
   Generic firmware linker script for STM32
*/

INCLUDE definitions.ld

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */

  /* The program code and other data goes into FLASH */
  .text :
  {
    FILL(0xFFFF)
      
    CREATE_OBJECT_SYMBOLS

    /* Bootloader code */
    KEEP(*(.bootloader))
      
    . = BOOTLOADER_SIZE; /* Set the start of the main program */
    _stext = .;          /* Provide the name for the start of this section */

    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    KEEP(*(.fwversiondata))

    INCLUDE common_text.ld

    . = ALIGN(4);
    _etext = .;   /* define a global symbols at end of code */
  } > FLASH

  INCLUDE common_sections.ld
}
