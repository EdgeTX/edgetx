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

  /* C++ comdat sections (templates, inline functions, operator
     new/delete) from the std:: namespace are instantiated by both UI
     and core objects, and the linker keeps a single copy whose home
     object depends on link order. That copy must stay callable at
     any time, so claim it here, before any target-specific pre-text
     section can grab it through file patterns (see
     pre_text_sections.ld). */
  .text.shared (READONLY) :
  {
    . = ALIGN(4);
    _stext_shared = .;
    *(.text._ZSt* .text._ZNSt* .text._ZNKSt* .text._ZN9__gnu_cxx*)
    *(.text._Znwj .text._Znwm .text._Znaj .text._Znam)
    *(.text._ZdlPv* .text._ZdaPv*)
    *(.rodata._ZSt* .rodata._ZNSt* .rodata._ZNKSt*)
    *(.rodata._ZTVSt* .rodata._ZTVNSt*)
    . = ALIGN(4);
    _etext_shared = .;
  } > REGION_TEXT AT> REGION_TEXT_STORAGE

  _text_shared_load = LOADADDR(.text.shared);

  /* Target-specific output sections that must claim their input
     sections before the .text catch-all (e.g. TX16S UI code
     relegated to flash bank 2, see
     stm32f429_sdram/pre_text_sections.ld) */
  INCLUDE pre_text_sections.ld

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

  /* Assets in FLASH only (REGION_ASSETS: same as REGION_TEXT_STORAGE
     on most targets, flash bank 2 on TX16S/F429) */
  .flash (READONLY) :
  {
    . = ALIGN(4);
    _sflash = .;
    *(.flash)

    . = ALIGN(4);
    _eflash = .;
  } > REGION_ASSETS

  .text_end_section : {} > REGION_TEXT AT > REGION_TEXT_STORAGE
  /* .flash is the highest LMA of the image on every target */
  _firmware_length = (LOADADDR(.flash) + SIZEOF(.flash)) - LOADADDR(.firmware_header);
  _firmware_version = _text_load;
}
