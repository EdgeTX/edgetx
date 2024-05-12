
.ccm (NOLOAD) :
{
  . = ALIGN(4);
  _sccm = .;
  *(.ccm)
  . = ALIGN(4);
  _eccm = .;
} > DTCMRAM

/* Reserve stack space in RAM_D3 */
.stack(NOLOAD) :
{
  . = ALIGN(4);
  . = . + MAIN_STACK_SIZE;
  . = ALIGN(4);
} > DTCMRAM

.iram (NOLOAD) :
{
  . = ALIGN(4);
  _siram = .;
  *(.iram)
  . = ALIGN(4);
  _eiram = .;
} > ITCMRAM AT> REGION_TEXT_STORAGE

_stext_iram = LOADADDR(.iram);

.sram2 (NOLOAD) :
{
  . = ALIGN(4);
  *(.sram2)
} > RAM_D2

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
} > SDRAM

PROVIDE(_heap_start = _esdram);
