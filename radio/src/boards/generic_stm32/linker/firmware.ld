/*
   Generic firmware linker script for STM32
*/

INCLUDE definitions.ld

/* Define output sections */
SECTIONS
{
  .bootloader :
  {
    FILL(0xFFFF)
    KEEP(*(.bootloader))
    . = BOOTLOADER_SIZE;
  } > REGION_BOOTLOADER

  /* ISR vector to be loaded */
  .isr_vector :
  {
    _sisr_vector = .;
    KEEP(*(.isr_vector))

    . = ALIGN(4);
    _eisr_vector = .;
  } > REGION_ISR_VECT AT> REGION_TEXT_STORAGE

  _isr_load = LOADADDR(.isr_vector);

  /* Main code section */
  .text :
  {
    FILL(0xFFFF)
    CREATE_OBJECT_SYMBOLS

    _stext = .;
    KEEP(*(.fwversiondata))

    INCLUDE common_text.ld

    . = ALIGN(4);
    _etext = .;
  } > REGION_TEXT AT> REGION_TEXT_STORAGE

  _text_load = LOADADDR(.text);

  INCLUDE common_sections.ld
}
