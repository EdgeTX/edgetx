
/* Highest address of the user mode stack */
STACK_ADDRESS = ORIGIN(RAM) + LENGTH(RAM); /* end of RAM */

/* Required amount of stack for interrupt stack (Main stack) */
MAIN_STACK_SIZE = 1024;

/* Maximum bootloader code size */
BOOTLOADER_SIZE = 0x8000;

/* Minimum Heap Size (indicative) */
MIN_HEAP_SIZE = 0;

/* Highest heap address */
HEAP_ADDRESS = STACK_ADDRESS - MAIN_STACK_SIZE;

/* Specify the memory areas */
MEMORY
{
  FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 512K
  RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 128K
}

REGION_ALIAS("REGION_BOOTLOADER",   FLASH);
REGION_ALIAS("REGION_ISR_VECT",     FLASH);
REGION_ALIAS("REGION_TEXT",         FLASH);
REGION_ALIAS("REGION_TEXT_STORAGE", FLASH);

REGION_ALIAS("REGION_DATA", RAM);
REGION_ALIAS("REGION_BSS", RAM);
REGION_ALIAS("REGION_RAM", RAM);
