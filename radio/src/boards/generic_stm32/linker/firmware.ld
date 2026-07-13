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

  /* Used only with H7 */
  .firmware_header (READONLY) :
  {
    KEEP(*(.fwdescription))
  } > REGION_TEXT_STORAGE
  
  /* ISR vector to be loaded */
  .isr_vector :
  {
    . = ALIGN(4);
    _sisr_vector = .;
    KEEP(*(.isr_vector))

    . = ALIGN(4);
    _eisr_vector = .;
  } > REGION_ISR_VECT AT> REGION_TEXT_STORAGE

  _isr_load = LOADADDR(.isr_vector);

  /* Hot interrupt-path code (USB device stack, MSC storage glue, SDIO
     diskio) executed from RAM: on some TX16S units, instruction
     fetches from the upper flash bank get corrupted under heavy
     IRQ/DMA load (USB crash, EdgeTX#5899); claimed before .text so
     these input sections land here. Requires non-LTO objects. */
  .fastcode :
  {
    . = ALIGN(4);
    _sfastcode = .;
    *ll_usb*(.text .text.*)
    *hal_pcd*(.text .text.*)
    *usb_driver*(.text .text.*)
    *usbd_conf*(.text .text.*)
    *usbd_core*(.text .text.*)
    *usbd_ioreq*(.text .text.*)
    *usbd_ctlreq*(.text .text.*)
    *usbd_msc*(.text .text.*)
    *usbd_storage_msd*(.text .text.*)
    *diskio_sdio*(.text .text.*)
    . = ALIGN(4);
    _efastcode = .;
  } > REGION_BSS AT> REGION_TEXT_STORAGE

  _fastcode_load = LOADADDR(.fastcode);

  /* Main code section */
  .text (READONLY) :
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

  /* Assets in FLASH only */
  .flash (READONLY) :
  {
    . = ALIGN(4);
    _sflash = .;
    *(.flash)

    . = ALIGN(4);
    _eflash = .;
  } > REGION_TEXT_STORAGE

  .text_end_section : {} > REGION_TEXT AT > REGION_TEXT_STORAGE
  _firmware_length = LOADADDR(.text_end_section) - LOADADDR(.firmware_header);
  _firmware_version = _text_load;
}
