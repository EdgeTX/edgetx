
/* Highest address of the user mode stack */
STACK_ADDRESS = ORIGIN(CCM) + LENGTH(CCM); /* end of CCM */

/* Required amount of stack for interrupt stack (Main stack) */
MAIN_STACK_SIZE = 1024;

/* Maximum bootloader code size */
BOOTLOADER_SIZE = 0x8000;

/* Minimum Heap Size (indicative) */
MIN_HEAP_SIZE = 0;

/* Highest heap address */
HEAP_ADDRESS = ORIGIN(RAM) + LENGTH(RAM);

/* Specify the memory areas */
MEMORY
{
  FLASH (rx) :
      ORIGIN = 0x08000000,
      LENGTH = DEFINED(__flash_size) ? __flash_size : 512K

  RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 128K
  CCM (xrw)       : ORIGIN = 0x10000000, LENGTH = 64K
}

REGION_ALIAS("REGION_BOOTLOADER",   FLASH);
REGION_ALIAS("REGION_ISR_VECT",     FLASH);
REGION_ALIAS("REGION_TEXT",         FLASH);
REGION_ALIAS("REGION_TEXT_STORAGE", FLASH);

REGION_ALIAS("REGION_DATA", CCM);
REGION_ALIAS("REGION_BSS", CCM);
REGION_ALIAS("REGION_RAM", RAM);
