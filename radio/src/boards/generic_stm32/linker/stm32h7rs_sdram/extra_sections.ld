
.ccm (NOLOAD) :
{
  . = ALIGN(4);
  _sccm = .;
  *(.ccm)
  . = ALIGN(4);
  _eccm = .;
} > RAM

/* Reserve stack space in RAM_D3 */
.stack(NOLOAD) : AT (ADDR(.stack))
{
  . = ALIGN(4);
  . = . + MAIN_STACK_SIZE;
  . = ALIGN(4);
} > RAM

.iram (NOLOAD) :
{
  . = ALIGN(4);
  _siram = .;
  *(.iram)
  . = ALIGN(4);
  _eiram = .;
} > RAM AT> REGION_TEXT_STORAGE

_stext_iram = LOADADDR(.iram);

.sram2 (NOLOAD) :
{
  . = ALIGN(4);
  *(.sram2)
} > RAM

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
} > PSRAM

PROVIDE(_heap_start = _esdram);
