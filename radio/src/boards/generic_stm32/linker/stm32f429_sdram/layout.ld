
/* Required amount of stack for interrupt stack (Main stack) */
MAIN_STACK_SIZE = 8192;

/* Maximum bootloader code size */
BOOTLOADER_SIZE = 0x20000;

/* Minimum Heap Size (indicative) */
MIN_HEAP_SIZE = 4096K;

/* SDRAM definitions */
SDRAM_START = DEFINED(__sdram_start) ? __sdram_start : 0xD0000000;
SDRAM_SIZE = DEFINED(__sdram_size) ? __sdram_size : 8192K;

/* Specify the memory areas */
MEMORY
{
  BOOTLOADER (rx) :
      ORIGIN = 0x08000000,
      LENGTH = BOOTLOADER_SIZE

  FLASH (rx) :
      ORIGIN = ORIGIN(BOOTLOADER) + LENGTH(BOOTLOADER),
      LENGTH = 2048K - LENGTH(BOOTLOADER)

  CCM (xrw) :
      ORIGIN = 0x10000000,
      LENGTH = 64K

  RAM (xrw) :
      ORIGIN = 0x20000000,
      LENGTH = 192K

  SDRAM(xrw) :
      ORIGIN = SDRAM_START,
      LENGTH = SDRAM_SIZE
}

/* Highest address of the user mode stack */
STACK_ADDRESS = ORIGIN(CCM) + LENGTH(CCM);

/* Highest heap address */
HEAP_ADDRESS = ORIGIN(SDRAM) + LENGTH(SDRAM);

REGION_ALIAS("REGION_DATA", RAM);
REGION_ALIAS("REGION_BSS", RAM);
REGION_ALIAS("REGION_RAM", RAM);
