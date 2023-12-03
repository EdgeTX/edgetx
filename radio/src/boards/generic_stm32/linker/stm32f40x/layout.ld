
/* Required amount of stack for interrupt stack (Main stack) */
MAIN_STACK_SIZE = 1024;

/* Maximum bootloader code size */
BOOTLOADER_SIZE = 0x8000;

/* Minimum Heap Size (indicative) */
MIN_HEAP_SIZE = 0;

/* Specify the memory areas */
MEMORY
{
  BOOTLOADER (rx) :
      ORIGIN = 0x08000000,
      LENGTH = BOOTLOADER_SIZE

  FLASH (rx) :
      ORIGIN = 0x08008000,
      LENGTH = DEFINED(__flash_size) ? __flash_size : 512K

  CCM (xrw) : ORIGIN = 0x10000000, LENGTH = 64K
  RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
}

/* Highest address of the user mode stack */
STACK_ADDRESS = ORIGIN(CCM) + LENGTH(CCM); /* end of CCM */

/* Highest heap address */
HEAP_ADDRESS = ORIGIN(RAM) + LENGTH(RAM);

REGION_ALIAS("REGION_DATA", CCM);
REGION_ALIAS("REGION_BSS", CCM);
REGION_ALIAS("REGION_RAM", RAM);
