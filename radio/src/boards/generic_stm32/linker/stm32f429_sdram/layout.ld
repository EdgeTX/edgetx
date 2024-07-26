
/* Highest address of the user mode stack */
STACK_ADDRESS = 0x10010000; /* end of 64K CCM */

/* Required amount of stack for interrupt stack (Main stack) */
MAIN_STACK_SIZE = 8192;

/* Maximum bootloader code size */
BOOTLOADER_SIZE = 0x20000;

/* Minimum Heap Size (indicative) */
MIN_HEAP_SIZE = 0;

/* SDRAM definitions */
SDRAM_START = DEFINED(__sdram_start) ? __sdram_start : 0xD0000000;
SDRAM_SIZE = DEFINED(__sdram_size) ? __sdram_size : 8192K;

/* Highest heap address */
HEAP_ADDRESS = SDRAM_START + SDRAM_SIZE;

/* Specify the memory areas */
MEMORY
{
  FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 2048K
  RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 192K
  CCM (xrw)       : ORIGIN = 0x10000000, LENGTH = 64K
  SDRAM(xrw)      : ORIGIN = SDRAM_START, LENGTH = SDRAM_SIZE
}

REGION_ALIAS("REGION_BOOTLOADER",   FLASH);
REGION_ALIAS("REGION_ISR_VECT",     FLASH);
REGION_ALIAS("REGION_TEXT",         FLASH);
REGION_ALIAS("REGION_TEXT_STORAGE", FLASH);

REGION_ALIAS("REGION_DATA", CCM);
REGION_ALIAS("REGION_BSS", RAM);
REGION_ALIAS("REGION_RAM", RAM);
