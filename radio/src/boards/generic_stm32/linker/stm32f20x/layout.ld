
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
      ORIGIN = ORIGIN(BOOTLOADER) + LENGTH(BOOTLOADER),
      LENGTH = 512K - LENGTH(BOOTLOADER)

  RAM (xrw) :
      ORIGIN = 0x20000000,
      LENGTH = 128K
}

/* Highest address of the user mode stack */
STACK_ADDRESS = ORIGIN(RAM) + LENGTH(RAM); /* end of RAM */

/* Highest heap address */
HEAP_ADDRESS = STACK_ADDRESS - MAIN_STACK_SIZE;

REGION_ALIAS("REGION_DATA", RAM);
REGION_ALIAS("REGION_BSS", RAM);
REGION_ALIAS("REGION_RAM", RAM);
