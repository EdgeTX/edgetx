/* PSRAM definitions */
PSRAM_START = DEFINED(__PSRAM_START__) ? __PSRAM_START__ : 0x90000000;
PSRAM_SIZE = DEFINED(__PSRAM_SIZE__) ? __PSRAM_SIZE__ : 32678K;

/* Highest heap address */
HEAP_ADDRESS = PSRAM_START + PSRAM_SIZE;

/* Highest address of the user mode stack */
STACK_ADDRESS = ORIGIN(DTCMRAM) + LENGTH(DTCMRAM);

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
  ITCMRAM (xrw)  : ORIGIN = 0x00000000, LENGTH = 4K
  DTCMRAM (xrw)  : ORIGIN = 0x20000000, LENGTH = 4K
  RAM (xrw)      : ORIGIN = 0x24000000, LENGTH = 456K
  PSRAM (rwx)    : ORIGIN = PSRAM_START, LENGTH = PSRAM_SIZE
  NORFLASH (rwx) : ORIGIN = 0x70000000, LENGTH = 128M 
}

REGION_ALIAS("REGION_BOOTLOADER",   FLASH);
REGION_ALIAS("REGION_TEXT_STORAGE", NORFLASH);
REGION_ALIAS("REGION_TEXT",         PSRAM);
REGION_ALIAS("REGION_ISR_VECT",     DTCMRAM);
REGION_ALIAS("REGION_DATA",         RAM);
REGION_ALIAS("REGION_BSS",          RAM);
REGION_ALIAS("REGION_RAM",          RAM);
