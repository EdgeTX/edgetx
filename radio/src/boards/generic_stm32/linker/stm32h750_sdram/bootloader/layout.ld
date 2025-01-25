/* SDRAM definitions */
SDRAM_START = DEFINED(__sdram_start) ? __sdram_start : 0xC0000000;
SDRAM_SIZE = DEFINED(__sdram_size) ? __sdram_size : 8192K;

/* Highest heap address */
HEAP_ADDRESS = SDRAM_START + SDRAM_SIZE;

/* Highest address of the user mode stack */
STACK_ADDRESS = ORIGIN(DTCMRAM) + LENGTH(DTCMRAM) - 16;    /* end of RAM */

/* Maximum bootloader code size */
BOOTLOADER_SIZE = 0x10000;

/* Required amount of stack for interrupt stack (Main stack) */
MAIN_STACK_SIZE = 8192;

/* Generate a link error if heap and stack don't fit into RAM */
MIN_HEAP_SIZE = 4096k;      /* required amount of heap  */

/* Specify the memory areas */
MEMORY
{
  FLASH (rx)     : ORIGIN = 0x08000000, LENGTH = 128K
  ITCMRAM (xrw)  : ORIGIN = 0x00000000, LENGTH = 64K
  DTCMRAM (xrw)  : ORIGIN = 0x20000000, LENGTH = 128K
  RAM_D1 (xrw)   : ORIGIN = 0x24000000, LENGTH = 512K
  RAM_D2 (xrw)   : ORIGIN = 0x30000000, LENGTH = 288K
  RAM_D3 (xrw)   : ORIGIN = 0x38000000, LENGTH = 64K
  SDRAM (rwx)    : ORIGIN = SDRAM_START, LENGTH = SDRAM_SIZE
}

REGION_ALIAS("REGION_TEXT_STORAGE", FLASH);
REGION_ALIAS("REGION_TEXT",         ITCMRAM);
REGION_ALIAS("REGION_ISR_VECT",     DTCMRAM);
REGION_ALIAS("REGION_DATA",         DTCMRAM);
REGION_ALIAS("REGION_BSS",          RAM_D1);
REGION_ALIAS("REGION_RAM",          RAM_D1);
