
/* Highest address of the user mode stack */
STACK_ADDRESS = 0x10010000; /* end of 64K CCM */

/* Required amount of stack for interrupt stack (Main stack) */
MAIN_STACK_SIZE = 8192;

/* Maximum bootloader code size */
BOOTLOADER_SIZE = 0x20000;

/* Minimum Heap Size (indicative) */
MIN_HEAP_SIZE = 0;

/* SDRAM definitions */
EXTRAM_START = DEFINED(__EXTRAM_START__) ? __EXTRAM_START__ : 0xD0000000;
EXTRAM_SIZE = DEFINED(__EXTRAM_SIZE__) ? __EXTRAM_SIZE__ : 8192K;

/* Highest heap address */
HEAP_ADDRESS = EXTRAM_START + EXTRAM_SIZE;

/* Specify the memory areas.

   The 2MB flash is split at the hardware bank boundary: on some
   STM32F42x units (flash rev < 3), USB D+ (PA12) switching noise
   corrupts reads from bank 2 (ST errata, EdgeTX#5899).  Restricting
   FLASH to bank 1 turns any overflow of code into a link error;
   only cold UI code/rodata and assets are placed in bank 2 (see
   pre_text_sections.ld) and must not be accessed while a USB
   session is active. */
MEMORY
{
  FLASH (rx)       : ORIGIN = 0x08000000, LENGTH = 1024K /* bank 1 */
  FLASH_BANK2 (rx) : ORIGIN = 0x08100000, LENGTH = 1024K /* bank 2 */
  RAM (xrw)        : ORIGIN = 0x20000000, LENGTH = 192K
  CCM (xrw)        : ORIGIN = 0x10000000, LENGTH = 64K
  SDRAM(xrw)       : ORIGIN = EXTRAM_START, LENGTH = EXTRAM_SIZE
}

REGION_ALIAS("REGION_BOOTLOADER",   FLASH);
REGION_ALIAS("REGION_ISR_VECT",     FLASH);
REGION_ALIAS("REGION_TEXT",         FLASH);
REGION_ALIAS("REGION_TEXT_STORAGE", FLASH);
REGION_ALIAS("REGION_ASSETS",       FLASH_BANK2);

REGION_ALIAS("REGION_DATA", CCM);
REGION_ALIAS("REGION_BSS", RAM);
REGION_ALIAS("REGION_RAM", RAM);
