/*
   Generic firmware linker script for STM32
*/

INCLUDE definitions.ld

/* Define output sections */
SECTIONS
{
  /* Bootloader code */
  .bootloader : {
    KEEP(*(.bootloader))
  } >BOOTLOADER AT>BOOTLOADER :text

  .bootloader_fill : {
    FILL(0xFFFF)
    . = ORIGIN(BOOTLOADER) + LENGTH(BOOTLOADER);
  } >BOOTLOADER AT>BOOTLOADER :text

  /* The program code and other data goes into FLASH */
  .text :
  {
    FILL(0xFFFF)
    CREATE_OBJECT_SYMBOLS

    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    KEEP(*(.fwversiondata))

    INCLUDE common_text.ld

    . = ALIGN(4);
    _etext = .;   /* define a global symbols at end of code */
  } > FLASH AT>FLASH :text

  INCLUDE common_sections.ld
}
