
INCLUDE section_ccm.ld

/* Reserve stack space in CCM */
.stack(NOLOAD) :
{
  . = ALIGN(4);
  . = . + MAIN_STACK_SIZE;
  . = ALIGN(4);
} > CCM AT>CCM :ram

.sdram(NOLOAD) :
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
} > SDRAM AT>SDRAM :ram

PROVIDE(__heap_start = _esdram);
