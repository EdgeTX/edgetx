
/* Task stacks */
.ccm (NOLOAD) :
{
  . = ALIGN(4);
  _sccm = .;
  *(.ccm)
  . = ALIGN(4);
  _eccm = .;
} > DTCMRAM

/* Reserve stack space */
.stack(NOLOAD) :
{
  . = ALIGN(4);
  . = . + MAIN_STACK_SIZE;
  . = ALIGN(4);
} > DTCMRAM

/* Fast code */
.iram :
{
  . = ALIGN(4);
  _siram = .;
  *(.iram)
  . = ALIGN(4);
  _eiram = .;
} > ITCMRAM AT> REGION_TEXT_STORAGE

_stext_iram = LOADADDR(.iram);

/* Disk cache buffers */
.disk_cache (NOLOAD) :
{
  . = ALIGN(4);
  *(.disk_cache)
} > PSRAM

/* Extra DMA section */
.dram (NOLOAD) :
{
  . = ALIGN(4);
  _s_dram = .;
  *(.dram)
  _e_dram = .;
} > DMA

_dram_addr = ADDR(.dram);

.sdram (NOLOAD) :
{
  . = ALIGN(4);
  *(.sdram)
  *(.sdram*)
  *(.sdram_rodata)
  *(.sdram_rodata*)

  . = ALIGN(4);
  _esdram = .;

  /* Reserve heap space in RAM */
  . = . + MIN_HEAP_SIZE;
  . = ALIGN(4);
} > PSRAM

PROVIDE(_heap_start = _esdram);
