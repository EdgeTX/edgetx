/*
*****************************************************************************
**
**  File        : stm32f4_flash.ld
**
**  Abstract    : Linker script for STM32F439 Device with
**                2MByte FLASH, 192KByte SRAM, 64KByte CCM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STMicroelectronics STM32
**
*****************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = 0x10010000;    /* end of 64K CCM */
_heap_end = 0xD0800000;  /* end of 8192K SDRAM */
__heap_end = _heap_end;

/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 4096K;      /* required amount of heap  */
_Main_Stack_Size = 8192; /* required amount of stack for interrupt stack (Main stack) */

/* Main stack end */
_main_stack_start = _estack - _Main_Stack_Size;

/* Specify the memory areas */
MEMORY
{
  BOOTLOADER (rx) : ORIGIN = 0x08000000, LENGTH = 128K
  FLASH (rx)      : ORIGIN = 0x08020000, LENGTH = 1920K
  RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 192K
  CCM (xrw)       : ORIGIN = 0x10000000, LENGTH = 64K
  SDRAM(xrw)      : ORIGIN = 0xD0000000, LENGTH = 8192K
}

PHDRS
{
  text PT_LOAD;
  ram PT_LOAD;
  ram_init PT_LOAD;
  tls PT_TLS;
}


/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .bootloader :
  {
    KEEP(*(.bootloader)) /* Bootloader code */
  } >BOOTLOADER AT>BOOTLOADER :text

  .bootloader_fill :
  {
    FILL(0xFFFF)
    . = ORIGIN(BOOTLOADER) + LENGTH(BOOTLOADER);
  } >BOOTLOADER AT>BOOTLOADER :text

  /* The program code and other data goes into FLASH */
  .text :
  {
    FILL(0xFFFF)
    CREATE_OBJECT_SYMBOLS
  
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    KEEP(*(.fwversiondata))
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    KEEP (*(SORT_BY_NAME(.init) SORT_BY_NAME(.init.*)))
    KEEP (*(.fini .fini.*))
    _etext = .;        /* define a global symbols at end of code */

    /* read-only data */
    *(.rdata)
    *(.rodata .rodata*)
    *(.gnu.linkonce.r.*)
    *(.srodata .srodata.*)
    *(.data.rel.ro .data.rel.ro.*)

    . = ALIGN(4);

    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);

    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
    KEEP (*(.init_array .ctors))
    PROVIDE_HIDDEN (__init_array_end = .);

    PROVIDE_HIDDEN ( __fini_array_start = . );
    KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
    KEEP (*(.fini_array .dtors))
    PROVIDE_HIDDEN ( __fini_array_end = . );

  } >FLASH AT>FLASH :text

  /*
   * Needs to be in its own segment with the PLT entries first
   * so that the linker will compute the offsets to those
   * entries correctly.
   */
  .got : {
    *(.got.plt)
    *(.got)
  } >FLASH AT>FLASH :text

  .toc : {
    *(.toc .toc.*)
  } >FLASH AT>FLASH :text

  /* used by the startup to initialize data */
  _sidata = .;

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
    *(.data .data*)
    *(.gnu.linkonce.d.*)

    . = ALIGN(4);
    *(.sdata .sdata.* .sdata2.*)
    *(.gnu.linkonce.s.*)
  } >RAM AT>FLASH :ram_init

  .tdata : ALIGN(__tls_align) {
    *(.tdata .tdata.* .gnu.linkonce.td.*)
    PROVIDE(__data_end = .);
    PROVIDE(__tdata_end = .);
  } >RAM AT>FLASH :tls :ram_init

  PROVIDE( __tls_align = MAX(ALIGNOF(.tdata),ALIGNOF(.tbss)) );

  PROVIDE( _sdata = ADDR(.data) );
  PROVIDE( _edata = __data_end );

  .tbss (NOLOAD) : {
    *(.tbss .tbss.* .gnu.linkonce.tb.*)
    *(.tcommon)
    PROVIDE( __tls_end = . );
    PROVIDE( __tbss_end = . );
  } >RAM

  PROVIDE( __tls_align = MAX(ALIGNOF(.tdata), ALIGNOF(.tbss)) );

  /* Uninitialized data section */
  .bss (NOLOAD) :
  {
    *(.sbss*)
    *(.gnu.linkonce.sb.*)
    *(.bss .bss*)
    *(.gnu.linkonce.b.*)
    *(COMMON)

    . = ALIGN(4);
    __bss_end = .;
  } >RAM

  PROVIDE( _sbss = ADDR(.tbss) );
  PROVIDE( _ebss = __bss_end );

  /* collect all uninitialized .ram sections */
  .ram (NOLOAD) :
  {
    . = ALIGN(4);
    _sram = .;
    *(.ram)
  } >RAM

  .ccm (NOLOAD) :
  {
    . = ALIGN(4);
    _sccm = .;
    *(.ccm)
    . = ALIGN(4);
    . = . + _Main_Stack_Size;
    _eccm = .;
  } >CCM

  .sdram (NOLOAD) :
  {
    *(.sdram)
    *(.sdram*)
    *(.sdram_rodata)
    *(.sdram_rodata*)
    _eram = .;
    . = ALIGN(4);
    . = . + _Min_Heap_Size;
    . = ALIGN(4);
  } >SDRAM

  PROVIDE ( end = _eram );
  PROVIDE ( _end = _eram );
  PROVIDE ( __heap_start = _end );
  
  /DISCARD/ :
  {
    /* Remove information from the standard libraries */
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )

    /* Throw away C++ exception handling information */
    *(.note .note.*)
    *(.eh_frame .eh_frame.*)
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    *(.ARM.exidx*)
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
