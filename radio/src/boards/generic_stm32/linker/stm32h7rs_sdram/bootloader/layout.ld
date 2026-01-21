/* PSRAM definitions */
EXTRAM_START = DEFINED(__EXTRAM_START__) ? __EXTRAM_START__ : 0x90000000;
EXTRAM_SIZE = DEFINED(__EXTRAM_SIZE__) ? __EXTRAM_SIZE__ : 32678K;

NORFLASH_START = 0x70000000;
NORFLASH_SIZE = 128M;

/* Highest heap address */
HEAP_ADDRESS = EXTRAM_START + EXTRAM_SIZE;

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
  FLASH (rx)     : ORIGIN = 0x08000000, LENGTH = 64K
  ITCMRAM (xrw)  : ORIGIN = 0x00000000, LENGTH = 64K
  DTCMRAM (xrw)  : ORIGIN = 0x20000000, LENGTH = 64K
  RAM (xrw)      : ORIGIN = 0x24000000, LENGTH = 384K
  DMA (xrw)      : ORIGIN = 0x24060000, LENGTH = 72K
  PSRAM (rwx)    : ORIGIN = EXTRAM_START, LENGTH = EXTRAM_SIZE
}

REGION_ALIAS("REGION_TEXT_STORAGE", FLASH);
REGION_ALIAS("REGION_TEXT",         ITCMRAM);
REGION_ALIAS("REGION_ISR_VECT",     DTCMRAM);
REGION_ALIAS("REGION_DATA",         RAM);
REGION_ALIAS("REGION_BSS",          RAM);
REGION_ALIAS("REGION_RAM",          RAM);
