
PROVIDE(_heap_start = _eram);

/* Reserve stack and heap space in RAM */
.stack(NOLOAD) :
{
  . = ALIGN(4);
  . = . + MIN_HEAP_SIZE;
  . = . + MAIN_STACK_SIZE;
  . = ALIGN(4);
} > REGION_RAM
