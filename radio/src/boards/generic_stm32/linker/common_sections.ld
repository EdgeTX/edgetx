
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

/* Initialized data sections goes into RAM, load LMA copy after code */
.data : {
  *(.data .data*)
  *(.gnu.linkonce.d.*)

  . = ALIGN(4);
  *(.sdata .sdata.* .sdata2.*)
  *(.gnu.linkonce.s.*)
} >REGION_DATA AT>FLASH :ram_init

/* Thread local storage */
.tdata : ALIGN(8) {
  *(.tdata .tdata.* .gnu.linkonce.td.*)
  PROVIDE(__data_end = .);
  PROVIDE(__tdata_end = .);
} >REGION_DATA AT>FLASH :tls :ram_init

PROVIDE(__data_start = ADDR(.data));
PROVIDE(__data_source = LOADADDR(.data));

PROVIDE(  _sdata = __data_start);
PROVIDE(  _edata = __data_end);
PROVIDE( _sidata = __data_source);

/* Thread local storage */
.tbss (NOLOAD) : {
  *(.tbss .tbss.* .gnu.linkonce.tb.*)
  *(.tcommon)
  PROVIDE( __tls_end = . );
  PROVIDE( __tbss_end = . );
} >REGION_BSS AT>REGION_BSS :ram

/* Uninitialized data section */
.bss(NOLOAD) : {
  *(.sbss*)
  *(.gnu.linkonce.sb.*)
  *(.bss .bss*)
  *(.gnu.linkonce.b.*)
  *(COMMON)

  . = ALIGN(4);
  __bss_end = .;
} >REGION_BSS AT>REGION_BSS :ram

PROVIDE( _sbss = ADDR(.tbss) );
PROVIDE( _ebss = __bss_end );

/* collect all uninitialized .ram sections (force DMA accessible) */
.ram (NOLOAD) :
{
  . = ALIGN(4);
  _sram = .;
  *(.ram)
  . = ALIGN(4);
  _eram = .;
} >REGION_RAM AT>REGION_RAM :ram

INCLUDE extra_sections.ld

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
